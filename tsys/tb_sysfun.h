#pragma once
//#include <string>
#include <tb_basetypes.h>
#include "tsys/tb_sysdecls.h"
#include "tb_exception_base.h"
#include "gdata/t_string.h"
#include "t_basedsync.h"

#define LOG_CREATE
#define LOG_CREATE_DLL

namespace crsys{





typedef void* HMODULE;
typedef void* HANDLE;

struct rSystemInfo_ex{
	bool filled;
	int CountOfProcessors;
	const char * ComputerName;
	const char * UserName;
};
struct rSystemInfo:public rSystemInfo_ex{
	rSystemInfo();
};

void Sleep(DWORD msecs);
time32_t getTickCount();



//HMODULE loadlibrary(const char* file);
HMODULE loadlibrary(const tbgeneral::param_stringA & file);
void* getprocaddress(HMODULE handle, const char* name);
int closelibrary(HMODULE handle);


struct r_memory_area_info {
	enum { pREAD=1, pWRITE=2, pEXECUTE=4 };
	uint options;
	byte* BaseAddress;
	std::uintptr_t regionsize; 
};

r_memory_area_info QueryMemoryRegion( void * v );


typedef int (SYSAPICALL * fn_ThreadProcType) ( LPVOID lpParam ); 

bool createthread( fn_ThreadProcType threadproc, void * param , HANDLE * hthread );
bool setthread_priority(int prior);
bool closethread(HANDLE * hthread);

inline bool createthread( fn_ThreadProcType threadproc, void * param , void * hthread ){ return createthread(  threadproc, param , (HANDLE *) hthread );};


}; //namespace crsys{

