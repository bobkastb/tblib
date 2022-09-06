#include "sync.h"

namespace tbgeneral{ namespace sync{

event_group_t::index_t wait_one( const wait_param_t & wp ){
	event_group_t g( &wp,1 ); 
	return g.wait();
};

event_group_t::index_t wait_multiple( const std::initializer_list<wait_param_t> & list ){
	event_group_t g(list ); 
	return g.wait();
}
event_group_t::index_t trywait_one( const wait_param_t & wp ){
	event_group_t g( &wp,1 ); 
	return g.wait( true );
};

event_group_t::index_t trywait_multiple( const std::initializer_list<wait_param_t> & list ){
	event_group_t g(list ); 
	return g.wait( true );
}

void event_group_t::test_duplicate(const wait_param_t * ar, size_t sz){
	for (size_t i=0;i<sz;i++) for (size_t j=i+1;j<sz;j++) {
		if (ar[i].eventp->get_source_id()==ar[j].eventp->get_source_id()) {
			throw "Duplicate event source in event_group_t";
		}
	}
};

void event_group_t::init( const wait_param_t * ar, size_t sz){
	test_duplicate( ar , sz);
	evlinks.resize(sz);
	for (size_t i = 0; i < sz; i++) {
		auto &lnk =evlinks[i]; auto & wp = ar[i];
		auto ev = wp.eventp;
		lnk.dataptr = wp.dataptr;
		lnk.operation = wp.operation;
		lnk.ev = wp.eventp; 
		lnk.waitgroup = this; 
		lnk.groupindex = i; 
	}
};

void event_group_t::init( event_t ** ar, size_t sz){
	evlinks.resize(sz);
	for (size_t i = 0; i < sz; i++) {
		auto & lnk = evlinks[i];
		lnk.operation = esyncOpEvent;
		lnk.ev = ar[i]; 
		lnk.waitgroup = this; 
		lnk.groupindex = i; 
	}
}

void event_group_t::link_events(){
	if (flinked_events) return; 
	flinked_events=true;
	for( auto & lnk : evlinks) {
		lnk.ev->link_group( &lnk  );
	}
};
void event_group_t::unlink_events(){
	if (!flinked_events) return; 
	flinked_events=false;
	for( auto & lnk : evlinks) {
		lnk.ev->unlink_group( &lnk );
	}
};

event_group_t::~event_group_t() { 
		AUTOLOCKER(lck, myevent.mu); // блокируем поступление сигналов!
		unlink_events();
}



bool event_base_t::popsignal( event_group_link_t * dest , event_group_link_t * source ){
	AUTOLOCKER(lck, *pmu);	
	return popsignal_nm(dest,source); 
};

void event_base_t::unlink_group( event_group_link_t * gl ){
	AUTOLOCKER(lck, *pmu);
	if ( gl==links ) { 
		links=gl->right;
	} else {
		if (gl->left) gl->left->right = gl->right;
		if (gl->right) gl->right->left = gl->left;
	}
	gl->left=gl->right=nullptr;
};

void event_base_t::link_group( event_group_link_t *  gl  ) { 
	AUTOLOCKER( lck , *pmu);
	if (!links) 
		links=gl;
	else { 
		gl->right = links; 
		links->left=gl; 
		links=gl; 
	}
};

void event_t::wait(){
	event_t* a[] = {this};
	auto r = wait_multiple( a );
};


bool event_t::popsignal_nm( event_group_link_t * dest , event_group_link_t * source){
	// здесь мы должны быть под мьютексом
	auto v=cnt_signals.fetch_add(-1);
	if (v<0) { cnt_signals.fetch_add(1); return false; }
	// здесь можно совершить операции из привязки
	if ( dest && binding_operation && !binding_operation( dest )) { 
		cnt_signals.fetch_add(1); return false; 
	}
	return true;
	
};

bool event_t::popsignal(event_group_link_t * dest , event_group_link_t * source){
	if (cnt_signals==0) return false;
	AUTOLOCKER(lck, *pmu);
	if (cnt_signals==0) return false;
	return popsignal_nm(dest,source);
};

void event_t::signal(int cnt){ 
	AUTOLOCKER(lck, *pmu);
	cnt_signals.fetch_add( cnt );
	signal_nm( nullptr );
}



int event_base_t::signal_nm( event_group_link_t * source , int sigflags ){ 
	int count=0;
	for (auto curr= links; curr; curr = curr->right) {
		if ((sigflags & esigfNoEmptyCheck)==0 && empty_event_nm()) break;
		if (curr->waitgroup->trysignalto( curr , source)) {
			count++;
			if (sigflags & esigfBreakOnFirst) break;
		} //else break; // тот же эффект что и на empty_event_nm()
	}
	return count;
}

bool event_group_t::trysignalto(  event_group_link_t * dest, event_group_link_t * source   ){
	if (waitstate!=estateWait ) return false; //TODO: это не просто ускорение, это блокировка второго входа в myevent.mu из того же потока во время OnCompleteWait
	AUTOLOCKER(lck, myevent.mu); // только один  event может подать сигнал!
	if (waitstate!=estateWait ) return false;
	if (!dest->ev->popsignal_nm(dest,source) ) return false;
	signal_index = dest->groupindex;
	waitstate = estateNoWait; 
	myevent.cv.notify_one(); // signal();
	return true;
}

int event_group_t::wait( bool try_wait ){
	// locker нельзя переносить ниже изза fdontBreakLinks. Пока есть "связи" popsignal должен накрываться locker-м
	AUTOLOCKER(lck , myevent.mu); 
	signal_index = esigNoSignal;
	for (auto &el : evlinks ) 
		if (el.ev->popsignal(&el,nullptr)) {
			signal_index= el.groupindex;
			return signal_index;
		}
	if (try_wait) 
		return signal_index;
	link_events();
	if (signal_index==esigNoSignal) {
		auto funisEndWait = std::bind(&event_group_t::isEndWait,this );
		signal_index = esigNoSignal;
		waitstate = estateWait;
		//myevent.cv.wait( lck , [&](){ return waitstate != estateWait; });
		myevent.cv.wait( lck ,  funisEndWait );
		waitstate = estateNoWait;
	}
	if (!fdontBreakLinks)
		unlink_events();
	//if (signal_index!=esigNoSignal) { 		auto gl = &evlinks[signal_index];		gl->ev->OnCompleteWait( gl );	}
	return signal_index;
};
bool event_group_t::isEndWait(){
	return waitstate != estateWait;
};

bool chan_base_t::do_operation_nm(eSyncOperation operation ,  void* data) {
	return operation==esyncOpReadChan ? readop_nm( data ): writeop_nm( data );
}

bool event_chan_t::popsignal_nm( event_group_link_t * dest , event_group_link_t * source){
	auto coevent = operation==esyncOpReadChan ? &fchan->fwriteevent : &fchan->freadevent;
	if ( fchan->isSyncChan() ) { // синхронный канал ( не буфферизованный maxsize=0 )
		if (!source) // нужно сразу попытаться подать сигнал, то бишь войти сюда повторно!
			return coevent->signal_nm( dest ,esigfNoEmptyCheck | esigfBreakOnFirst )!=0;
		else { 	// здесь можно делать синхронное read/write
				// здесь просто нужно копировать данные, все остальное уже сделано!
			if (operation==esyncOpReadChan)
				fchan->sync_rw( dest->dataptr , source->dataptr );
			else 
				fchan->sync_rw( source->dataptr , dest->dataptr );
			return true;
		}
	} else { // асинхронный канал ( буфферизованный maxsize>0 )
		auto res=  fchan->do_operation_nm( operation , dest->dataptr );
		if (!source) // проверка на res не делается . Неважно успешно чтение/запись или нет - запись/чтение можно делать в обоих случаях!
			coevent->signal_nm( dest  ); // source!=0 не пройдет, не стоит плодить рекурсию!
		return res;
	}

	// для синхронного chan надо зайти  
};
bool chan_base_t::empty_event_nm( eSyncOperation operation) { 
	return operation==esyncOpReadChan ? count_in<=0 : count_in>=maxsize; 
};

bool event_chan_t::empty_event_nm() { 
	return fchan->empty_event_nm( operation ); 
};

/*
void event_chan_t::OnCompleteWait( event_group_link_t * gl ){	
	return;
	LENTER(lck , *pmu);
	auto coevent = operation==esyncOpReadChan ? &fchan->fwriteevent : &fchan->freadevent;
	auto coop = operation==esyncOpReadChan ? esyncOpWriteChan : esyncOpReadChan ;
	if (!fchan->emptyfor(coop))
		coevent->signal_nm( nullptr );
};
*/

/* ev->popsignal(&el)
* Для синхронного канала (не буфферизованный канал)
*  lock(src group) -> lock (chan(events)) -> lock(dst group)
*  <src group> != <dst group> В одной группе нельзя ждать chan два раза
*  здесь свой код... Значит 
*  signal_nm , trysignalto = должны иметь лямбда функцию
*  для синхронного chan по сути не используется empty, потому что он всегда false !!
*/

}}; // namespace tbgeneral{ namespace sync{