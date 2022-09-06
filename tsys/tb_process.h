#pragma once

#include "gdata/t_string.h"


namespace crsys{

typedef int ProcessId_t;
struct rProcess{
	enum {EF_GetOutStream=1,EF_ASYNC , EF_NoLinkStream =1<<2, 
		tm_STEP=50 , to_INFINITE=(uint)-1 ,   };
	// --------- in
	uint flags;
	int ProcessBasedPriority;
	ProcessId_t pid;
	int * stopsignal;
	tbgeneral::stringA setdir; // set this dir as current for process
	uint timeout;
	// -------------- out 
	FILE* outstrm; // out
	int status; // process exitcod
	rProcess() { init0(); }
	rProcess(uint _flags) { init0(); flags=_flags; }
	~rProcess() { if (outstrm) fclose(outstrm);  };
	uint waitstep() { return tm_STEP;}
	bool checktimeout(uint tm) { if(!this) return false; return (timeout==to_INFINITE || tm<timeout) ? false : true; }
	int SysExecute( const char * exepath , const char * argv[] , int argcnt );
	int SysExecute( const char * execmd );

	uint tm_step() { return tm_STEP;};
	protected:
		void init0() {ZEROMEM(*this);timeout=(uint)-1;}
	};
inline int SysExecute( const char * exepath , const char * argv[] , int argcnt ) { return rProcess().SysExecute(exepath , argv , argcnt); };
int SysExecuteDef( const char * exepath , const char * argv[] , int argcnt );
int getpid();

void printprgmemSize();
}; // namespace crsys

