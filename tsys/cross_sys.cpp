#define _CROSS_SYS_COD


//#include <precompiled.h>
#include <stdlib.h>
#include <stdarg.h>
#include "cross_sys.h"
#include "tb_sysfun.h"
#include "systime.h"
#include "sys_mutex.h"
#include "tb_log.h"
#include "tbfiles.h"
#include "tsys/sys_resource.h"
#include "gdata/tb_c_tools.h"
#include "gdata/t_string.h"
#include "parse/t_path.h"
#include "tsys_helper.h"


#include <vector>
//#include <string>
//HMODULE xxx;

#ifdef _WIN32
#include <time.h>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
//#include <Winsock2.h>

#else  // linux


#include <dlfcn.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/utsname.h>

#include <pthread.h>
#include <errno.h>

#endif
//#include "communicator_mutex.h"

warning_MSVC(disable , 4996)



//{ char LOG[1030]; tb_snprintf(LOG, 1024, __VA_ARGS__); for (char* p=LOG; *p; ++p) if (*p<0) *p=koi8enc[0x7F&*p]; printf("%s\n", LOG); }


void panic_error_do(const char x , const char * _file, int _linen   ){
	LOG("panic error! at %s line %d exit!" , _file , _linen );
    exit(0); 
};








namespace crsys{
static void fill_rSystemInfo( rSystemInfo & r );

rSystemInfo::rSystemInfo(){
	ZEROMEM(*this);
	fill_rSystemInfo( *this );
	};


};






//==========================
/*
void __attribute__((constructor)) initialize_dll() {
		if (rDLLStart::main) rDLLStart::main->init_fin_proc[0]();
}
void __attribute__((constructor)) finalize_dll() {
		if (rDLLStart::main) rDLLStart::main->init_fin_proc[1]();
}
*/


namespace crsys{
int32 _InterlockedDecrement(int32 * d){ return _InterlockedAdd(d,-1); };
int32 _InterlockedIncrement(int32 * d){ return _InterlockedAdd(d,1); };

#ifdef _WIN32 

int32 _InterlockedAdd(int32 * d , int32 v){
	return (int32) InterlockedExchangeAdd((long*)d,v);
};

#else // ========== LINUX -=========

int32 _InterlockedAdd(int32 * d , int32 v){
	return __sync_fetch_and_add( d , v ); 
};

#endif
};

#ifdef _WIN32 



#ifndef _WIN64

//#ifdef _WINDLL
//#pragma comment(linker, "/include:__mySymbol")
//rDLLStart* rDLLStart::main;
//#pragma comment(linker, "/include:?initialize@@YAXXZ")
//voidproc init_fin_proc[2] = { initialize,finalize };
 //initialize,finalize 
//extern voidproc init_fin_proc[2] = { initialize,finalize };
//__declspec( selectany ) extern

//extern "C" { int _afxForceUSRDLL; } 

extern "C" uint32 _tls_index;
static bool initialize_tls=0;

// байда 
//TODO:
BOOL WINAPI __DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	enum { TLSsize=1024 };
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH: {
		initialize_tls = _tls_index ==0 ;
		//LOG("DLL TLS=%d \n",_tls_index);
		if (initialize_tls) {_tls_index = TlsAlloc();
		//	_tls_index = 1;
		};
		//initialize_dll();
	}; break;
	case DLL_THREAD_ATTACH: if (initialize_tls) {
		void * tlsdata = malloc( TLSsize ); 
		memset( tlsdata , 0 , TLSsize ); 
		TlsSetValue(_tls_index ,  tlsdata);
		//_asm mov edx , fs:[0x2c]
		__asm{
			mov edx , fs:[0x2c]
			mov ecx, _tls_index
			mov eax, tlsdata
			mov [edx+ecx*4] , eax	
		}
	}; break;
	case DLL_THREAD_DETACH: if (initialize_tls) {
		free(TlsGetValue(_tls_index));
		__asm{
			mov edx , fs:[0x2c]
			mov ecx, _tls_index
			mov [edx+ecx*4] , 0	
		};

	};break;
	case DLL_PROCESS_DETACH: {
		if (initialize_tls) TlsFree(_tls_index);
	}break;
	};

	return 1;
}
//#endif
#endif //#ifndef _WIN64




namespace crsys{
typedef ::HMODULE w_HMODULE;
typedef ::HANDLE w_HANDLE;
void Sleep(DWORD msecs) { ::Sleep(msecs);};
time32_t getTickCount() { return ::GetTickCount();};
/*
HMODULE loadlibrary(const char* file){
	HMODULE* h= (HMODULE*)LoadLibraryA(file);
	DWORD err= GetLastError();
	return h;
};
*/

HMODULE loadlibrary(const tbgeneral::param_stringA & file) {
	//char bb1[256]; file.param.tocstr(bb1, sizeof(bb1));
	wchar_t buff[256];
	auto wfn = tbgeneral::buffWfromAstr(file, buff);
	HMODULE* h = (HMODULE*)LoadLibraryW(wfn.c_str());
	DWORD err = GetLastError();
	return h;
}
void* getprocaddress(HMODULE handle, const char* name){
	return GetProcAddress((w_HMODULE)handle, name);
};
int closelibrary(HMODULE handle){
	return FreeLibrary((w_HMODULE)handle);
};

bool createthread( fn_ThreadProcType threadproc, void * param , HANDLE * hthread ){
//bool createthread( void* threadproc, void * param , HANDLE * hthread ){
	DWORD thid; HANDLE temp; 
	temp = (HANDLE) CreateThread( 0 , 0 , (LPTHREAD_START_ROUTINE) threadproc , param , 0 , &thid );
	if ( hthread ) *hthread = temp;
	return temp != INVALID_HANDLE_VALUE;
};

bool closethread(HANDLE * hthread){
	return 0!=CloseHandle(*hthread);
};



/*
bool isDebuggerPresent(void){ 
	return false;
	// return 0!=IsDebuggerPresent();
};
*/

bool setthread_priority(int prior){
	//THREAD_PRIORITY_ABOVE_NORMAL = 1 , NORMAL = 0
	if (prior<=-2) prior=THREAD_PRIORITY_IDLE;

	return 0!=SetThreadPriority( GetCurrentThread() , prior );
//if (!SetThreadPriority( GetCurrentThread() , THREAD_PRIORITY_ABOVE_NORMAL )) 	 printf("Error at SetThreadPriority!!!!!!!!!!!!!!!!!!!!!!!!!!!----------------\n");
};

static void fill_rSystemInfo( rSystemInfo & r ){
	SYSTEM_INFO sysinfo;
	static char currCompName[64]={0};
	GetSystemInfo( &sysinfo );
	r.CountOfProcessors = sysinfo.dwNumberOfProcessors;
	r.ComputerName = currCompName;
	if (!currCompName[0]) {
		DWORD sz=sizeof(currCompName);
		GetComputerNameA(currCompName, &sz);
	};
	//TODO:
};


r_memory_area_info QueryMemoryRegion( void * v ){
	enum { pREAD=r_memory_area_info::pREAD, pWRITE=r_memory_area_info::pWRITE, pEXECUTE=r_memory_area_info::pEXECUTE };
	r_memory_area_info res; ZEROMEM(res);
	MEMORY_BASIC_INFORMATION rec;
	if (VirtualQuery(  v,   &rec,   sizeof(rec) )) {
		res.BaseAddress = (byte*) rec.AllocationBase; 
		res.regionsize = ((byte*)rec.BaseAddress + rec.RegionSize) - res.BaseAddress;
		auto ap= rec.AllocationProtect;
		switch (ap & 0xff) {
		case PAGE_EXECUTE: res.options = pEXECUTE; break;
		case PAGE_EXECUTE_READ: res.options = pEXECUTE | pREAD ; break;
		case PAGE_EXECUTE_READWRITE: res.options = pEXECUTE | pREAD | pWRITE; break;
		case PAGE_EXECUTE_WRITECOPY: res.options = pEXECUTE | pREAD | pWRITE; break;
		case PAGE_READONLY: res.options = pREAD ; break;
		case PAGE_READWRITE: res.options = pREAD | pWRITE ; break;
		case PAGE_WRITECOPY: res.options = pREAD | pWRITE ; break;
		}
	}
	return res;
}


bool testPointerInConstSegment( const void* pdata ) {
	struct MMemInfo{ enum{ PAGESIZE=4*1024, PAGeOFSMASK=0xFFF }; typedef std::vector<char> maptype;
		char* abase; size_t size;
		std::vector<char> * map;
		static size_t getpagecnt( size_t sz){ return (sz/PAGESIZE) + (sz % PAGESIZE?1:0);}	
		size_t cntReadOnlyPages;
	};
	static MMemInfo mymeminf={0};
	if (!mymeminf.abase) { AUTOENTERG(); if (!mymeminf.abase) {
		static MEMORY_BASIC_INFORMATION rec;
		char* st="ssss"; uint regsz=0;	uint res= static_cast<uint>(VirtualQuery(  st,   &rec,   sizeof(rec) ));
		mymeminf.abase =(char*) rec.AllocationBase; 
		struct cpinf{ char* start; size_t size; }; std::vector<cpinf> map;	
		for (st = mymeminf.abase,rec.RegionSize=0 ; rec.AllocationBase == mymeminf.abase ;st=(char*)rec.BaseAddress + rec.RegionSize) {	
			if (!VirtualQuery(  st,   &rec,   sizeof(rec) )) break;
			mymeminf.size = ((char*)rec.BaseAddress + rec.RegionSize) - mymeminf.abase;
			switch ( rec.Protect & 0xFF) { 
			case PAGE_READONLY: case PAGE_EXECUTE_READ: { 
				cpinf rr={(char*)rec.BaseAddress , rec.RegionSize}; 
				map.push_back(rr); };break;
			};
		};
		mymeminf.map=new MMemInfo::maptype(); auto & resmap = *mymeminf.map; 
		resmap.resize( 1 + MMemInfo::getpagecnt(mymeminf.size)/8 ); 
		memset( resmap.data(), 0 , resmap.size() );
		for (size_t i=0;i<map.size();i++ ) { auto e=map[i]; 
			size_t pgs = MMemInfo::getpagecnt ( e.size ); 
			size_t sti=MMemInfo::getpagecnt ( e.start - mymeminf.abase ); 
			mymeminf.cntReadOnlyPages+=pgs;
			for (; pgs; sti++,pgs--) { 
				auto& ref=resmap[sti>>3]; 
				ref=ref | (1<<(sti&7)); 
			};
		};
	}};
	if (pdata < mymeminf.abase) return false;
	if (pdata >= (mymeminf.abase+mymeminf.size) ) return false;
	//if ( ( (char*)(rec.BaseAddress) <= (char*)pdata ) && ( (char*)pdata < ((char*)(rec.BaseAddress)+rec.RegionSize) )) return true;
	if (mymeminf.map) { size_t pg=((char*)pdata-mymeminf.abase)>>12; 
			if ((*mymeminf.map)[pg>>3] & (1<<(pg&7))) 
					return true;
			else	return false; 
	};	
	return true;
};

}; // namespace crsys



//static SYSTEMTIME LocalTime11;



#else  // ========== LINUX -=========


/*
int __stdcall gettimeofday(struct timeval* tv, void* timezone) {
	if (tv) {
		tv->tv_sec=_time32(0);
		tv->tv_usec=0;
	}

	return 0;
}
*/

namespace crsys{

bool testPointerInConstSegment( const void* pdata ) {
	//TODO: get for linux
	#pragma warning "out of testPointerInConstSegment function"
	return false;
};


bool add_path_to_LIBENV(const char * x){
	auto prgpath = tbgeneral::get_file_dir(x)+":"; 
	if (prgpath.size()>2){ char * eP = getenv("LD_LIBRARY_PATH");
		char * s= strstr(eP , prgpath.c_str() );
		if (!(s && ((s==eP) || (s[-1]==':')))) {
			prgpath << eP;
			setenv("LD_LIBRARY_PATH", prgpath.c_str() , 1 );
			return true;
		};
	};
	return false;
} 

bool add_paths_to_LIBENV(){
	static bool added=false;
	if (added) return false;
	added = true;
	add_path_to_LIBENV(".");
	add_path_to_LIBENV(rfilesystem::get_programm_path().c_str());
	LOG("change envc .'LD_LIBRARY_PATH'='%s'\n", getenv("LD_LIBRARY_PATH"));
	return true;
};


bool testloadlibrary(const char* path , const char* file, HMODULE & mod , std::string & fnm ){
	fnm = std::string(path) +(strlen(path)?"/":"") +file;
	mod=dlopen(fnm.c_str(), RTLD_NOW );
	LOG("testload library >%s\n" , fnm.c_str() );
	return mod!=0;
	//bool lazy=!hmod;if (lazy) hmod=dlopen(fnm.c_str(), RTLD_LAZY);
}

//HMODULE loadlibrary(const char* file){
HMODULE loadlibrary(const tbgeneral::param_stringA & _file) {
	char bb1[256]; auto files = _file.param.tocstr(bb1, sizeof(bb1));
	auto file = files.c_str();

	//add_paths_to_LIBENV();
	std::string fnm , err;HMODULE	hmod=0;bool lazy=false;
	if (!hmod) testloadlibrary( rfilesystem::get_programm_dir().c_str() , file , hmod , fnm);
	if (!hmod) testloadlibrary( "." , file , hmod , fnm);
	if (!hmod) testloadlibrary( "" , file , hmod , fnm);
	
	//printf("loadlibrary.0");
	LOG(" loading library '%s' - %s err=%s", fnm.c_str() , hmod ? (lazy ? "warning:lazy" : "ok") : "failed" , dlerror() );
	//printf("loadlibrary.1");
	return hmod;
};

void* getprocaddress(HMODULE handle, const char* name){
	return dlsym( handle, name);
};
int closelibrary(HMODULE handle){
	return dlclose(handle);
};

void Sleep(DWORD msecs) {
	usleep(msecs*1000);
}

time32_t getTickCount() {
    tms tm;
	DWORD TPerS=sysconf(_SC_CLK_TCK);
    return (times(&tm)*1000) / TPerS;
}


static void fill_rSystemInfo( rSystemInfo & r ){
	//TODO:
	static rSystemInfo_ex res={0};
	static utsname unm;
	if (!res.filled) { 	AUTOENTERG();	if (!res.filled) {
		res.CountOfProcessors = sysconf(_SC_NPROCESSORS_ONLN);
		uname(&unm); 
		res.ComputerName = unm.nodename;
		res.filled=true;
	};	memcpy( &r , &res , sizeof(res));
	} else 	memcpy( &r , &res , sizeof(res));
	//tsname utsn; if (uname(&utsn)>=0) strncpy(compname, utsn.nodename, TSIZE(compname));

};

typedef void *(* fn_thread__start_routine) (void *);

bool createthread( fn_ThreadProcType threadproc, void * param , HANDLE * hthread ){
	HANDLE __hthread;	if (!hthread) hthread =&__hthread;	*hthread =0;
	bool res= 0==pthread_create( (pthread_t *)hthread , 0 , (fn_thread__start_routine)threadproc , param );
	if (!res) RAISE("on pthread_create( (pthread_t *)hthread , 0 , threadproc , param )");
	return res;
};
bool closethread(HANDLE * hthread){
	int res; pthread_t h=*(pthread_t*) hthread[0]; 
	//	pthread_join () или pthread_detach () 
	LOG("THREAD:wait thread_join %p", h);
	res = pthread_join( h , 0 );
	LOG("THREAD: thread_join( %p )=%d  repeat=%d ",h,res,pthread_join( h , 0 ));
	return res==0;
};


bool setthread_priority(int prior){
	return false;
	int policy, retcode ;
	pthread_t threadID=  pthread_self();    struct sched_param param;
    if ((retcode = pthread_getschedparam(threadID, &policy, &param)) != 0)
    {   errno = retcode;
        perror("pthread_getschedparam");
		return false;
      //  exit(EXIT_FAILURE);
    };


	//printf("SCHED_RR:%d %d\n" , sched_get_priority_min(SCHED_RR), sched_get_priority_max(SCHED_RR)); 
	//printf("SCHED_OTHER:%d %d\n" , sched_get_priority_min(SCHED_OTHER), sched_get_priority_max(SCHED_OTHER)); 
	//printf("PrintPriority: th=%p policy=%s (%d)  priority=%d \n",	threadID,((policy == SCHED_FIFO)  ? "SCHED_FIFO" : (policy == SCHED_RR)    ? "SCHED_RR" : (policy == SCHED_OTHER) ? "SCHED_OTHER" :"???"), policy,param.sched_priority		);
	
	param.sched_priority=prior>0 ? prior*10 : 0 ;
	//param.sched_priority=0;
    if ((retcode = pthread_setschedparam(threadID, SCHED_RR , &param)) != 0)
    {   errno = retcode;
        perror("pthread_setschedparam");
		return false;
     //   exit(EXIT_FAILURE);
    };
	return true;
};

//bool isDebuggerPresent(void){ return false; };



}; // namespace crsys

#endif


//-----------------------------------------------------------------
//#include "communicator_mutex.h"

#ifdef _WINDOWS
class CMutex { 
public:
	void enter() { EnterCriticalSection(&fcs);  };
	void leave() { LeaveCriticalSection(&fcs); };
	CMutex() { InitializeCriticalSection( &fcs ); };
	~CMutex() { DeleteCriticalSection(&fcs); };
private:
	CRITICAL_SECTION fcs;
  };


#else

//#include <pthread.h>

class CMutex { 
public:
	void enter() { pthread_mutex_lock(&fcs); };
	void leave() { pthread_mutex_unlock(&fcs); };
	CMutex() { 
		pthread_mutexattr_init( &attr );
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);

		pthread_mutex_init(&fcs, &attr ); };
	~CMutex() { 
		pthread_mutex_destroy(&fcs); 
		pthread_mutexattr_destroy(&attr);	
	};
private:
	pthread_mutexattr_t attr;
	pthread_mutex_t fcs;
  };

#endif

namespace crsys{
//template<> cResourceDistrib<CMutex> * cResourceDistrib<CMutex>::ResourceDistrib;

void sys_mutex_base::enter(){
	((CMutex*)data)->enter();
};
void sys_mutex_base::leave(){
	((CMutex*)data)->leave();
};
//sys_mutex general_mutex;
sys_mutex::sys_mutex() 
{ data = new CMutex(); };
sys_mutex::~sys_mutex(){
  delete (CMutex*)data; data=0;
};
sys_mutex_c::sys_mutex_c() 
{ data = AllocateGlobalCashedResource<CMutex>(); };
sys_mutex_c::~sys_mutex_c(){
	FreeGlobalCashedResource((CMutex*)data);
	data=0;
};

};// namespace crsys{

namespace crsys{
#ifdef _WINDOWS
//extern wchar_t ** _environ;
//extern wchar_t ** _wenviron;
const char** get_all_environment_sysA() { return const_cast<const char**>(_environ); }
const wchar_t** get_all_environment_sysW() { return const_cast<const wchar_t**>_wenviron; }
#else
const char** get_all_environment_sysA() { return const_cast<const char**>( environ); }
const wchar_t** get_all_environment_sysW() { return 0; }
//extern char ** environ;
#endif
}; // namespace crsys{

enum {MUTEX_END_NUMBER=32};
static CMutex mutex_array[MUTEX_END_NUMBER+1]; 
//CMutex * mutex_array;
static CMutex* get_general_mutex(){
	static CMutex  _general_mutex;
	return &_general_mutex;
}

void __tmp_mutex_l( int lock , int index ) {
	if (size_t(index)>=arraylen(mutex_array) || index<0 ) 
		RAISE("mutex index out of range");
	CMutex * m = mutex_array + index;
	//Sleep(1);
	if (lock) m->enter(); else m->leave();
};
entrleave_scope_real::entrleave_scope_real( int mutex_id ){
	if (size_t(mutex_id)>=arraylen(mutex_array)) {
		RAISE("mutex index out of range");
	};
	init<CMutex>( &mutex_array[mutex_id] );
};

entrleave_scope_real::entrleave_scope_real(){
	init<CMutex>( get_general_mutex() );
	//LOG("entrleave_scope_real::entrleave_scope_real()");
}; // general data copy mutex

//tbgeneral::stringA rr;

//#error "A C++ compiler is required!"
