#define _CROSS_SYS_COD

#include "tb_sysfun.h"
#include "tsys/tb_service.h"
//#include "tsys/systime.h"
#include "tsys/sys_mutex.h"
#include "tsys/tb_log.h"
#include "tsys/tbfiles.h"
#include "gdata/t_format.h"
#include "parse/t_path.h"


//#undef RAISE
#ifndef _WIN32
#include <unistd.h>
#include <pthread.h>
#include <syslog.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/resource.h>
#endif

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
//#include <Advapi32.h>
//#include <wtsapi32.h>
#endif

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-result"
#endif





namespace crsys{

#ifdef _WIN32

bool IsWorkStationLocked()
{
    HDESK hDesk = OpenInputDesktop(0, FALSE, DESKTOP_READOBJECTS);
    if (hDesk == NULL)
    return TRUE;

    TCHAR szName[80];
    DWORD cbName;
    BOOL bLocked;
    
    bLocked = !GetUserObjectInformation(hDesk, UOI_NAME, szName, 80, &cbName)
          || lstrcmpi(szName, TEXT("default")) != 0;

    CloseDesktop(hDesk);
    return bLocked!=0;
}
#else
	bool IsWorkStationLocked() { return true; }
#endif
//---------------------------------------------------

#ifndef _WIN32
//----------------------------------------------- LINUX SERVICE ---------------------------
 

 

/*
static void _close_all_handles(){
        struct rlimit limits;
        if ( getrlimit(RLIMIT_NOFILE, &limits) < 0 ) {
                perror("can`t get RLIMIT_NOFILE");
                exit(EXIT_FAILURE);
        }
        if ( limits.rlim_max == RLIM_INFINITY )
                limits.rlim_max = 1024;
 
        u_int32_t idx;
        for ( idx = 0; idx < limits.rlim_max; ++idx ) {
                close(idx);
        }
};
*/




static void demonize(r_service_process & srv) {
	    //const char* cmd = srv.servicename.c_str();
        pid_t pid;
        struct sigaction sa;
 
        umask(0);
 
 
        if ( (pid = fork()) < 0 ) 
			{ perror("fork error"); exit(EXIT_FAILURE); } 
		else if ( 0 != pid ) 
			exit(0);
 
        if ( (setsid()) == (pid_t)-1 ) 
			{ perror("setsig error"); exit(EXIT_FAILURE); }
 
        sa.sa_handler = SIG_IGN;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        if ( sigaction(SIGHUP, &sa, NULL) < 0 ) 
			{ perror("can`t ignore SIGHUP"); exit(EXIT_FAILURE); }
 
        if ( chdir("/") < 0 ) 
			{ perror("can`t chdir() to /"); exit(EXIT_FAILURE); }
 
 		//_close_all_handles();

}
 
/***************************************************************************/
 
static int already_running(r_service_process & srv) {
	    const char* cmd = srv.servicename.c_str();

        char buff[4096] = "\0";
        sprintf(buff, "%s%s", "/var/run/", cmd);
        /**  */
        int fd = open(buff, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        if ( fd < 0 ) {
                perror("can`t open PID file");
                exit(EXIT_FAILURE);
        }
        if ( lockf(fd, F_TLOCK, 0) ) {
                if ( errno == EACCES || errno == EAGAIN ) {
                        close(fd);
                        return 1;
                }
                perror("can`t lock PID file");
                exit(EXIT_FAILURE);
        }
 
        ftruncate(fd, 0);
        sprintf(buff, "%d", (int)getpid());
        write(fd, buff, strlen(buff));
 
        return 0;
}
 
/***************************************************************************/
static sigset_t mask;
 
static void* process_signal_thr(void* arg) {


		printf("enter signal thread\n");
        r_service_process* opt = (r_service_process*)arg;
		printf("enter wait signal \n");
        while ( opt->running ) {
                int signo = 0;
                if ( 0 != sigwait( &mask, &signo) ) 
					{ syslog(LOG_ERR, "error in sigwait: %s", strerror(errno)); exit(EXIT_FAILURE); }
                switch ( signo ) {
                        case SIGHUP:
                                syslog(LOG_INFO, "reread configuration");
                                printf( "reread configuration \n");
                                opt->onRefresh(); 
                                break;
                        case SIGTERM:
                                printf( "receive signal SIGTERM. exiting...\n");
                                syslog(LOG_INFO, "receive signal SIGTERM. exiting...");
                                opt->onStop(); 
                                break;
						case SIGCHLD: // завершение дочернего процесса
								break;
                        default:
                                syslog(LOG_INFO, "unspecified signal '%d' received. ignored.", signo);
                                printf( "unspecified signal '%d' received. ignored.\n" , signo);
                                break;
                }
        }
        return NULL;
}
 // http://www.cyberforum.ru/cpp-linux/thread194108.html#post1125774


int r_service_process::callAsService(){
        demonize(*this);

        if ( already_running(*this) ) 
		{ perror("daemon already running"); exit(EXIT_FAILURE); }

		redirect_stdio();
        running = 1;
 
		{
        struct sigaction sa;

        sa.sa_handler = SIG_DFL; sigemptyset(&sa.sa_mask);  sa.sa_flags = 0;
        if ( sigaction(SIGHUP, &sa, NULL) < 0 ) 
			{ perror("sigaction error"); exit(EXIT_FAILURE); }
 
        sigfillset(&mask);
        if ( 0 != pthread_sigmask(SIG_BLOCK, &mask, NULL) ) 
			{ perror("pthread_sigmask error");exit(EXIT_FAILURE); }
		};
        pthread_t tid;
        if ( pthread_create(&tid, NULL, process_signal_thr, this ) ) {
                perror("can`t create thread");
                exit(EXIT_FAILURE);
        }
		return mainloop();
};

#endif


// ------------------------ WINDOWS SERVICE

#ifdef _WIN32



static DWORD WINAPI ServiceCtrlThread(void *param ){
	r_service_process & self = *(r_service_process*) param;
	//std::string("Global\\")+ this->programmname+".stopevent";
	tbgeneral::stringA fn = strlower( tbgeneral::get_file_name(filesystem::get_programm_path()) );
	
	tbgeneral::stringA evname = tbgeneral::format("Global\\%s.stopevent" , fn );
	HANDLE ev= OpenEventA(  EVENT_ALL_ACCESS ,  false ,  evname.c_str() );
	if (ev==0) 
	{  LOG_ERROR("Event not opened '%s' (service not running) ", evname.c_str()  );
	   return 0;
	};
	LOG("DO Wait event: '%s' ", evname.c_str());
	WaitForSingleObject( ev , INFINITE );

	LOG("Stop signal from %s", evname.c_str());
	self.onStop();
		
	return 0;
};

void r_service_process::parsecmd(){ 
	servicename = tbgeneral::get_file_name(filesystem::get_programm_path().c_str()); 
};

int r_service_process::callAsService(){
	redirect_stdio();
	running=1;
	//DispatchTable[0].lpServiceName = (char*) systemservicename.c_str();
	//CurrService.self=this;
	HANDLE temp = (HANDLE) CreateThread( 0 , 0 ,  ServiceCtrlThread , this , 0 , 0 );	CloseHandle(temp);
	return mainloop();

};


#endif


void r_service_process::redirect_stdio(){
	stringA sio[]={ tbgeneral::stringA( getSTDfiles(1)) , tbgeneral::stringA( getSTDfiles(2) )};
	filesystem::reopen_std_handles(0, sio[0].c_str(),sio[1].c_str());
};


};//namespace crsys;

