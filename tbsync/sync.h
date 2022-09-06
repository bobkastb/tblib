#pragma once

#include <condition_variable>
#include <thread>
#include <chrono>
#include <iostream>
#include <vector>
#include <queue>
#include <functional>

//TODO: нужен shared_ptr для event_group_link_t
// массив внутри event_group_t должен иметь внутренний буфер на 16 event_group_link_t ...
// empty_event_nm для синхронных каналов - нужно возвращать три значения а не 2, или добивать ему в параметры src,dst: event_group_link_t

namespace tbgeneral{ namespace sync{

class event_group_t; 
//class event_t;
class event_base_t;
class chan_base_t;
template <typename EType = int> class chan_t;

enum eSyncOperation{ esyncOpEvent , esyncOpReadChan , esyncOpWriteChan };

struct event_group_link_t {
	event_base_t * ev=nullptr; // здесь нужен shared ptr
	event_group_t * waitgroup=nullptr; 
	int groupindex=-1;
	eSyncOperation operation=esyncOpEvent;
	void * dataptr= nullptr;
	event_group_link_t *left=nullptr, *right=nullptr; // groups links chain for one Event
}; 

//define LENTER( lck , mu ) std::unique_lock<std::mutex> lck(mu)
#if __cplusplus >= 201703L  // c++17
#define AUTOLOCKER( lck , mu ) std::unique_lock lck(mu)
#else  
template<typename Tmu> std::unique_lock<Tmu> _makeuniquelock(Tmu & mu) { return std::unique_lock<Tmu>(mu); }
#define AUTOLOCKER( lck , mu ) auto lck = _makeuniquelock( mu )
#endif

class event_base_t  {
	friend class event_group_t;
	friend class chan_base_t;
protected:
	enum eSignalFlags{ esigfNoEmptyCheck=1, esigfBreakOnFirst=2  };

	event_group_link_t * links=0;
	std::recursive_mutex * pmu=nullptr;

	// все эти вызовы из  под lock. *_nm - не вызывают lock внутри, остальные вызывают
	void link_group( event_group_link_t *  gl  );
	void unlink_group( event_group_link_t * gl );
	virtual bool empty_event_nm()=0; // в момент вызова event_group_link_t не синхронизирован (не залочен)
	//source may be nullptr!!
	virtual bool popsignal( event_group_link_t * dest , event_group_link_t * source );
	virtual bool popsignal_nm( event_group_link_t * dest , event_group_link_t * source )=0;
	int signal_nm( event_group_link_t * source , int sigflag=0 ); // source - только для каналов (связанные события)
public:
	virtual uintptr_t get_source_id()=0;// { return (uintptr_t)this ;};

};

class event_t : public event_base_t{
	friend class event_group_t;
	friend class chan_base_t;
	std::atomic<int32_t> cnt_signals=0;
	std::function<bool(event_group_link_t *)> binding_operation; 
protected:
	std::recursive_mutex _mu;
	bool popsignal_nm( event_group_link_t * dest , event_group_link_t * source );
	bool popsignal( event_group_link_t * dest , event_group_link_t * source ) override;
	bool empty_event_nm() override { return cnt_signals<=0; };
public: 
	uintptr_t get_source_id() override { return (uintptr_t)this;};
	event_t(){ pmu= &_mu;}
	void wait();
	void signal(int cnt=1);
	void set();
	void clear();
};

class event_chan_t : public event_base_t {
	protected:
	friend class chan_base_t;
	chan_base_t * fchan;
	eSyncOperation operation;
	bool popsignal_nm( event_group_link_t * dest , event_group_link_t * source) override;
	bool empty_event_nm() override;
	void init(eSyncOperation _op , std::recursive_mutex * _pmu , chan_base_t * _fchan ) { 
		operation = _op; pmu = _pmu; fchan=_fchan;
	}; 
	void OnCompleteWait( event_group_link_t * gl );
public:
	uintptr_t get_source_id() override { return (uintptr_t)fchan;};
};

class chan_base_t {
	friend class event_chan_t;
	protected:
	std::recursive_mutex mu;
	event_chan_t freadevent;
	event_chan_t fwriteevent;
	int32_t maxsize=0;
	int32_t count_in=0;
	chan_base_t() { 
		freadevent.init(  esyncOpReadChan,  &mu , this ); 
		fwriteevent.init( esyncOpWriteChan, &mu , this );  
	}
	bool isSyncChan() { return maxsize==0; }
	bool empty_event_nm( eSyncOperation operation);
	virtual bool readop_nm( void* data)=0;
	virtual bool writeop_nm( const void* data)=0;
	virtual bool sync_rw( void* dest , const void* source )=0;
	bool do_operation_nm(eSyncOperation operation ,  void* data);
	public:
	event_base_t* get_event(eSyncOperation operation) { return operation==esyncOpReadChan ?  &freadevent : &fwriteevent; };
};



class wait_param_t {
public:
	eSyncOperation operation;
	event_base_t * eventp;
	void * dataptr= nullptr;
	wait_param_t( event_t* ev){
		operation = esyncOpEvent; eventp = ev; dataptr=nullptr ;
	}
	template<typename T> wait_param_t( eSyncOperation op , chan_t<T>* chan , T* dp){
		operation = op; eventp = chan->get_event(op);	dataptr=dp;
	};
};


template <typename EType>
class chan_t : public chan_base_t {
	//using EType = int;
	std::queue<EType> que;
protected:
	bool sync_rw( void* dest , const void* source ) override {
		if (!dest ) return false;
		auto dp = static_cast<EType*>(dest); auto sp=static_cast<const EType*>(source);
		dp[0] = sp ? sp[0] : EType();
		return true;
	};
	bool readop_nm( void* data) override {
		bool res=false;
		if (que.size()) {
			if (data) {
				auto dp = static_cast<EType*>(data);
				*dp = que.front();	
			};
			que.pop();
			res=true;
		}
		count_in = que.size();
		return res;
	};
	bool writeop_nm( const void* data) override{
		bool res=false;
		if (static_cast<int32_t>(que.size())<maxsize) {
			if (data) {
				auto dp = static_cast<const EType*>(data);
				que.push( *dp );
			} else 
				que.push( EType() );
			res=true;
		}
		count_in = que.size();
		return res;
	};

public: 
	chan_t(size_t _maxsize=0) {maxsize=_maxsize;}
	bool trypush(const EType & v);
	bool trypop( EType & v);
	bool push(const EType & v){
		EType vv= v;
		wait_param_t wp[]={ {esyncOpWriteChan, this , &vv } };
		return wait_multiple( wp );
	};
	bool pop( EType & v){
		wait_param_t wp[]={ {esyncOpReadChan, this , &v } };
		return wait_multiple( wp );
	};

};




class event_group_t { // single thread object
	struct event_ll_t {
		std::mutex mu;
		std::condition_variable cv;
		//int32_t sig=0;
		//void signal( ){ сv.notify_one(); }
		//void wait(){LENTER(lck , mu); cv.wait( lck , [&](){ return sig!=0; });	};
	};
	friend class event_base_t;
public:
	using index_t = int;
	enum { esigNoSignal=-1 , estateNoWait=0, estateWait=1};
private:
	event_ll_t myevent;
	std::vector<event_group_link_t> evlinks;
	int32_t waitstate=estateNoWait;
	int32_t signal_index=esigNoSignal;
	bool flinked_events=false; // state for link_events|unlink_events
	bool fdontBreakLinks=false; // не разрывать связи каждый раз при выходе из ожидания
protected:
	bool trysignalto(  event_group_link_t * dest , event_group_link_t * source );
private:
	void init( event_t ** ar, size_t sz);
	void init( const wait_param_t * ar, size_t sz);
	void link_events();
	void unlink_events();
		// сигналы одинаковых источников запрещены.(throw)
	void test_duplicate(const wait_param_t * ar, size_t sz);
public:
	//event_group_t(){ };
	event_group_t( const std::initializer_list<wait_param_t> & list ){	init( list.begin() , list.size() );	};
	event_group_t(const wait_param_t * ar, size_t sz){ init(ar,sz); };
	event_group_t(const wait_param_t & wp){ init(&wp,1); };
	event_group_t(event_t ** evpp, size_t sz){ init(evpp,sz); };
	//template <int... wpa> event_group_t( ...wpa ){  };
	
	~event_group_t();
	int wait( bool try_wait=false);
	bool isEndWait();
	bool mode_dontBreakLinks() { return fdontBreakLinks;}
	bool mode_dontBreakLinks(bool val) { auto res=fdontBreakLinks; fdontBreakLinks=val; return res;}
}; 

template <size_t Sz> event_group_t::index_t wait_multiple( event_t* (&ar ) [Sz]){
	event_group_t g(&ar[0], Sz ); 
	return g.wait();
};
template <size_t Sz> event_group_t::index_t wait_multiple( const wait_param_t (&ar ) [Sz]){
	event_group_t g(&ar[0], Sz ); 
	return g.wait();
};
event_group_t::index_t wait_multiple( const std::initializer_list<wait_param_t> & list );
event_group_t::index_t wait_one( const wait_param_t & wp );
event_group_t::index_t trywait_multiple( const std::initializer_list<wait_param_t> & list );
event_group_t::index_t trywait_one( const wait_param_t & wp );

}}; // namespace tbgeneral{ namespace sync{