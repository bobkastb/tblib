#define _T_PROCESS_CPP_

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-result"
#pragma GCC diagnostic ignored "-Wnonnull-compare"

#endif

#ifdef __GNUC__
#define warning_GCC(cmd,id) _Pragma("GCC diagnostic " #cmd " \"-W" id "\"")
#else 
#define warning_GCC(cmd,id)
#endif



//warning_GCC(ignored ,"unused-but-set-variable")
//warning_GCC(ignored,"nonnull-compare")


#define _CROSS_SYS_COD
#include "cross_sys.h"
#include "tb_sysfun.h"
#include "tb_process.h"
#include "tbfiles.h"
#include "sys_mutex.h"
#include "parse/t_parse.h"
#include "parse/t_path.h"

// -Wunused-but-set-variable

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#include <process.h>

#else
#include <unistd.h>
#include <spawn.h>
#include <wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <malloc.h>


#endif





namespace crsys{

struct cProgrammMemoryInfo{
	typedef long tpagescount;
	typedef uint64 tbytescount;
	uint whodefined;
	tbytescount total;
	tbytescount resident;
	tbytescount shared;
	tbytescount heapsize;
	tbytescount heapblocks;
	void getAll();
};

int SysExecuteDef( const char * exepath , const char * argv[] , int argcnt ) { 
	rProcess p;
	p.flags |= rProcess::EF_NoLinkStream;
	return p.SysExecute(exepath , argv , argcnt); 
};



#ifdef _WINDOWS
int getpid(){
	return _getpid();
};


void printprgmemSize()
{	cProgrammMemoryInfo mi; mi.getAll();
	printf("MEMCONTROL: heapblocks:%d heapsize=%ld vsize=%I64d  \n", uint(mi.heapblocks), uint(mi.heapsize) , mi.total  );
	//printf("testlong: %ld" , uint64(1000000)*1000000 );
}
void cProgrammMemoryInfo::getAll(){
	ZEROMEM(*this);
	_CrtMemState st;
	_CrtMemCheckpoint(   &st );
	size_t fullcntb=0;
	for (size_t i=0;i<arraylen( st.lCounts);i++) fullcntb +=st.lCounts[i];
	heapsize = fullcntb;
	heapblocks=st.lTotalCount;
	whodefined = whodefined | (3 << 16);
};


int rProcess::SysExecute( const char * execmd){
	
	//SECURITY_ATTRIBUTES sa;
	int stopsignal_stuff=0 , *ss=( stopsignal ? stopsignal : &stopsignal_stuff );
	this->status=0;
	PROCESS_INFORMATION pi;
	STARTUPINFOA si; auto si_p=&si;
	tbgeneral::stringA childnms[2] , _execmd=execmd ;

	tbgeneral::parseline_result rcmdl;
	rcmdl= tbgeneral::parsecmdline(execmd);
	if (setdir.size()==0) setdir= tbgeneral::get_file_dir(rcmdl.list[0]);
	
	uint ProcPriority=0;
	

	filesystem::FILEauto child_in; 
	ZEROMEM(si);
	si.cb          = sizeof(si);
	if ((flags & EF_NoLinkStream)==0){
	child_in.d= filesystem::makeTempFile("tmp" , childnms[0]);
	this->outstrm	= filesystem::makeTempFile("tmp" , childnms[1]);
		ProcPriority= this->ProcessBasedPriority < 0 ? IDLE_PRIORITY_CLASS : ( this->ProcessBasedPriority > 0 ? HIGH_PRIORITY_CLASS : 0 );
	si.dwFlags     = STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
	si.wShowWindow = SW_HIDE;
	si.hStdInput   = (HANDLE) _get_osfhandle(_fileno(child_in.d));
	si.hStdOutput  = flags & EF_GetOutStream  ? (HANDLE) _get_osfhandle(_fileno(this->outstrm)) : 0;
	si.hStdError   = si.hStdOutput;
	} 

	bool rv =0!= CreateProcessA(NULL,  _execmd.data() ,/* Command line */
		NULL, NULL,	/* Proc & thread security attributes */
		TRUE,		/* Inherit handles */
		ProcPriority ,	/* Creation flags */
		NULL,		/* Environment block */
		this->setdir.c_str(),		/* Current directory name */
		si_p, &pi);
	if (rv) {
		CloseHandle(pi.hThread);
		uint tm;bool isterm; 
		for( tm=0,isterm=false ; ((ss[0]&0x100)==0)&&(!isterm)&&(!checktimeout(tm)) ; tm+=tm_step() ) {
			isterm = WAIT_TIMEOUT != WaitForSingleObject( pi.hProcess , tm_step());
		};
		if ( isterm ) GetExitCodeProcess(pi.hProcess,(DWORD*)&status);
		CloseHandle(pi.hProcess);
	};
	if (this->outstrm) fseek(this->outstrm,0,0);
	return rv ? status : GetLastError() ;

};
int rProcess::SysExecute( const char * exepath , const char * argv[] , int argcnt ){
		tbgeneral::stringA s=exepath;
		for (int i=0;i<argcnt;i++) s<<" "<<argv[i];
		return SysExecute( s.c_str() );
};

#elif defined(_LINUX)

int getpid(){
	return ::getpid();
};

static uint64 getProgramVsize(cProgrammMemoryInfo * pi=0) {
	char buff[128];  long l=0 , res,share, psize = sysconf(_SC_PAGESIZE);
	sprintf(buff, "/proc/%d/statm", (int)getpid());
	FILE * f= fopen( buff, "rb" );
	fscanf( f, "%ld", &l ); fscanf( f, "%ld", &res ); fscanf( f, "%ld", &share ); 
	fclose(f);
	//printf("read file (%s) size=%d  pagesize=%d" , buff , uint(l) , (uint)sysconf(_SC_PAGESIZE));
	if (pi) { pi->total = l * psize;
		pi->resident = res * psize; pi->shared = share * psize;
		pi->whodefined = pi->whodefined | 3;
	};
	return l* psize;

};
void cProgrammMemoryInfo::getAll(){
	ZEROMEM(*this);
	struct mallinfo mi;  mi = mallinfo();
	getProgramVsize(this);
	heapsize = mi.hblkhd + mi.uordblks;	
	heapblocks = 0;
	whodefined = whodefined | (1 << 16);
};

void printprgmemSize()
{	cProgrammMemoryInfo mi; mi.getAll();
	printf("MEMCONTROL: heapblocks:UNDEF heapsize=%ld vsize=%ld self=%ld \n", mi.heapsize , mi.total , mi.total-mi.shared );
}


int rProcess::SysExecute( const char * execmd){
  tbgeneral::parseline_result r;
  r= tbgeneral::parsecmdline(execmd);
  return this->SysExecute( r.list[0] , (const char **) &r.list[1] , r.list.size()-1 );
}

int rProcess::SysExecute( const char * exepath , const char * argv[] , int argcnt ) 
{ tbgeneral::darray<const char *> args;tbgeneral::darray<char *> Env;
  tbgeneral::stringA outsfn;
  int stopsignal_stuff=0 , *ss=( stopsignal ? stopsignal : &stopsignal_stuff );
  this->status=0;
  pid_t pid;
  //{ printf("------- args ----\n"); for (int i=0;i<argcnt;i++) printf("arg[%d]='%s'; ",i,argv[i]);	printf("\n");  }
  args.resize(argcnt+1);
  memcpy(&args[1],argv,argcnt*sizeof(args[0]));
  args[0]=exepath;
  if (flags & EF_GetOutStream)  {
		outstrm= rfilesystem::makeTempFile("tmp",outsfn);
	  };
  if (setdir.size()==0) setdir=tbgeneral::get_file_dir(exepath);
  //outsfn="/tmp/stdouttst.txt";
  //if (outsfn.size()) 	{ outsfn.push_front('>'); args.push_back(outsfn.c_str()); };
  
  args.push_back(0);
  posix_spawn_file_actions_t sfa;
  posix_spawn_file_actions_init( &sfa );
  if (outsfn.size()) {	int res;
  
	res=posix_spawn_file_actions_addclose(&sfa, 1);
	res=posix_spawn_file_actions_addopen(&sfa, 1, outsfn.c_str() , O_NONBLOCK|O_WRONLY , S_IWRITE );
	res=posix_spawn_file_actions_addclose(&sfa, 2);
	res=posix_spawn_file_actions_addopen(&sfa, 2, outsfn.c_str() , O_NONBLOCK|O_WRONLY , S_IWRITE );
    //printf("set stdout to '%s' res=%d\n",outsfn.c_str(),res);
  };	
  int err=0;
  static tbgeneral::stringA orgcdir;
  {  AUTOENTERG(); //TODO: 
		if (!orgcdir.size()) orgcdir = rfilesystem::getCurrentDir();
		chdir( this->setdir.c_str() );
	err=posix_spawn(&pid, exepath , &sfa , 0 ,(char**) &args[0] , 0 ); 
		chdir( orgcdir.c_str() );
  };

  this->pid=pid;
  //printf("------------- new process run %d & pid=%d ------------ \n",err , pid);
  if (err>=0) {
	int res; uint tm=0, tm_step=waitstep(); bool isterm=false;
	while((ss[0]&0x100)==0) {
		res=waitpid(pid, &status, WNOHANG);
		if (res==pid) { isterm=true; break; };
		crsys::Sleep(tm_step); tm+= tm_step;
		if (checktimeout(tm)) break;
	};
	if (!(flags & EF_ASYNC) && !isterm) {
		printf("DEBUG:TIMEOUT!!!!!!!\n");
		kill(pid, SIGKILL);
		waitpid(pid, &status,0);
	};
  }; // if (err>0)	
  return err>=0 ? status : err ;	
	//printf("************* end of waitpid*************\n");
}; // SysExecute


#endif 


}; // namespace crsys

//warning_GCC(ignored ,"unused-but-set-variable")


