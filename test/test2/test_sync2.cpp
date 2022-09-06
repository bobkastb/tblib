#include "tbsync/sync.h"
//-------------------- CPP --------------

using namespace tbgeneral::sync;

struct wctx_t{
	event_t e1,e2,e3;
	chan_t<int> chsync[3];
	chan_t<int> chasync[3]={chan_t<int>(1),chan_t<int>(1),chan_t<int>(1)};
	//wctx_t(){};
};


/* //template <typename EType > ( event_t* (&ar ) [Sz])
template <typename EType , size_t Sz > std::ostream& operator << ( std::ostream& os , EType (&ar ) [Sz]){
	for (auto & e : ar){
		os << " " << e;
	}
	return os;
};
*/


using namespace std::chrono_literals;
void waits_th( wctx_t * ctx){
	int rvals[]= {0,0,0,0};
	event_t* ea[] = { &ctx->e1, &ctx->e2, &ctx->e3	};
	{	event_group_t wg( {&ctx->e1, &ctx->e2, &ctx->e3} ); //event_group_t wg1(ea);
		event_group_t wg1 = {&ctx->e1, {esyncOpReadChan , &ctx->chsync[0] , rvals+0} , &ctx->e3} ;
	}
	wait_param_t wp[] = { &ctx->e1 , {esyncOpReadChan , &ctx->chsync[0] , rvals+0} , {esyncOpReadChan , &ctx->chsync[1],rvals+1} , {esyncOpReadChan , &ctx->chsync[2],rvals+2} };
	std::cout << "wait signal...\n";
	std::cout << wait_multiple( ea ) << "\n";
	std::cout << wait_multiple( ea ) << "\n";
	std::cout << wait_multiple( ea ) << "\n";
	
	//std::cout << multi_wait( ea ) << "\n";
	for (int i=0;i<3;i++) {
		//std::this_thread::sleep_for( 100ms );
		std::cout << "Schan:" << wait_multiple(wp) << ": " << rvals[0] << " " << rvals[1] << " " << rvals[2] << "\n";
	};
	wait_param_t wp1[] = { {esyncOpReadChan , &ctx->chasync[0] , rvals+0} , {esyncOpReadChan , &ctx->chasync[1],rvals+1} , {esyncOpReadChan , &ctx->chasync[2],rvals+2} };
	for (int i=0;i<3;i++) {
		std::cout << "Achan:" << wait_multiple(wp1) << ": " << rvals[0] << " " << rvals[1] << " " << rvals[2] << "\n";
	};

	std::cout << "found signal...\n";
}
void signal_th(wctx_t * ctx){
	//return;
	std::this_thread::sleep_for( 1000ms );
	std::cout << "<< signal 0!\n";
	ctx->e1.signal();
	std::cout << "<< signal 1!\n";
	ctx->e2.signal();
	std::cout << "<< signal 2!\n";
	ctx->e3.signal();
	std::cout << "<< push 11 to chan 1!\n";
	//std::this_thread::sleep_for( 500ms );
	for (int i=0;i<3;i++)
		ctx->chsync[i].push(11+i);
	for (int i=0;i<3;i++)
		ctx->chasync[i].push(21+i);
	std::cout << "do signal!\n";
}


void mainsync_test1(){
	auto vcpp = __cplusplus;
	wctx_t  ctx;
	std::thread t1(waits_th,&ctx), t2(signal_th,&ctx);
	t1.join(); 
	t2.join();
};