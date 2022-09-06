#define _CROSS_SYS_COD
#include "tsys/cross_sys.h"
#include "tsys/tb_ssockets.h"
#include "gdata/tb_algo.h"
//#include "tsys/systime.h"
#include "tsys/sys_mutex.h"
#include "tsys/tb_log.h"
#include "tb_exception_base.h"

//tbgeneral::stringA rr;
//#include "gdata/tb_log.h"


#ifdef USE_BOOST

#include <precompiled.h>
#include <boost/asio.hpp>

namespace crsys{

struct c_b_ssocket: public c_ssocket{
	boost::asio::io_service ios;
	std::auto_ptr<boost::asio::ip::tcp::socket> fsocket;
	virtual ~c_b_ssocket() 
		{ fsocket.reset(0); };
	virtual bool connect(){
		boost::asio::ip::tcp::resolver resolver(ios);
		boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(), ip.c_str(), port_c.c_str());
		boost::asio::ip::tcp::resolver::iterator iterator=resolver.resolve(query);
		fsocket.reset(new boost::asio::ip::tcp::socket(ios));
		fsocket->connect(*iterator);
		return fsocket->is_open();

	};
	virtual int write( const void * data , uint size){
		size_t ret=boost::asio::write(*fsocket, boost::asio::buffer( data , size ));
		return (int)ret;
		};  
	virtual int read( void * data , uint size){
		size_t ret=boost::asio::read(*fsocket, boost::asio::buffer( data , size ));
		return (int)ret;
		};  
	virtual int settimeout( uint to_type , uint ms ){
		struct timeval tv;
		tv.tv_sec = ms;	//таймаут получения данных блокирующего сокета - 5 минут
		tv.tv_usec = 0 ;
		return setsockopt(fsocket->native(), SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));
	}; 
	virtual bool is_open(){
		return fsocket->is_open();	
	};
};

c_ssocket* c_ssocket::newsocket(){
	return new c_b_ssocket();
};
#endif


#ifdef _WINDOWS
	#include "winsock2.h"
	#pragma comment(lib, "Ws2_32.lib" )
	
	static bool start_sockets(){
		WSADATA wsaData; static bool started=false;
		if (started) return true;;
		int iResult = WSAStartup(MAKEWORD(2,2), &wsaData); //
		if (iResult != NO_ERROR)
			crsys::eraise("Error at WSAStartup()\n");
		started=true;
		return iResult == NO_ERROR;
	};
	static uint getlastsockerror(){
		return WSAGetLastError();
	}
	static bool CheckSockRes( const SOCKET & s ) { return s!=INVALID_SOCKET; }
	static int set_socket_timeout( SOCKET s, uint totype, uint ms )  {
		// SO_RCVTIMEO , SO_SNDTIMEO
		return setsockopt(s, SOL_SOCKET, totype ,(char*) &ms, sizeof(ms));
	};
	
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>


	
	typedef int SOCKET;
	//typedef struct sockaddr_in sockaddr_in; 
	typedef struct sockaddr SOCKADDR;
	static bool start_sockets(){ return 1;};
	static uint getlastsockerror(){ return 0;};
	static bool CheckSockRes( const SOCKET & s ) { return s>=0; }
	static int set_socket_timeout( SOCKET s, uint totype, uint ms )  {
		struct timeval tv;
		//int rt;
		tv.tv_sec = (ms/1000)+ (ms% 1000?1:0); //ms/1000;	//таймаут получения данных блокирующего сокета - 5 минут
		tv.tv_usec = 0; // % 1000;
		return setsockopt(s, SOL_SOCKET, totype, (char*)&tv, sizeof(tv));
	};
	#define SOCKET_ERROR -1
	static void closesocket( const SOCKET & s ) { close(s); };
#endif


namespace crsys{
struct c_triv_ssocket: public c_ssocket{
	SOCKET sock;
	bool isconnection;
	c_triv_ssocket() {
		sock = SOCKET(-1); isconnection=false;
		foptions |= eopt_ReadSize_MAX;
	};
	virtual int native() { return (int)sock;}
	virtual ~c_triv_ssocket()	{ 
		if (CheckSockRes(sock)) closesocket(sock); 
	};
	virtual bool init( const char * _port , const char * ip){ 
		start_sockets();
		c_ssocket::init(_port , ip); isconnection=false;
		if (!CheckSockRes( sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))) {
			eraise("c_triv_ssocket.init() Error at socket(): %d \n", getlastsockerror());
			return false;
		}
		return true; 
	};
	virtual bool connect(){
		struct sockaddr_in clientService; 
		clientService.sin_family = AF_INET;
		clientService.sin_addr.s_addr = inet_addr( this->ip.c_str() );
		clientService.sin_port = htons( atoi( this->port_c.c_str() ) );
		if ( ::connect( sock, (SOCKADDR*) &clientService, sizeof(clientService) ) <0) 
			eraise("c_triv_ssocket.connect() Error at connect(): %d \n", getlastsockerror());
		isconnection=true;
		return true;
	};
	virtual int write( const void * data , uint size){ if (size==0) return 0;
            int r=::send(sock, (char*)data , size , 0);
			if (r<=0) eraise("c_triv_ssocket.write() failed \n");
			return r;
	}; 
	virtual int read( void * data , uint size ){
			char * buff = (char*)data; uint csz;int bytes_read=0;
			for (csz=size;csz>=0; buff+=tbgeneral::max(bytes_read,0)) {
				bytes_read = ::recv(sock, buff, csz , 0);
				csz -= tbgeneral::max(bytes_read,0);
				if(bytes_read == 0) { 
					isconnection=false; 
					if (csz==size) eraise("c_triv_ssocket.read() connection closed \n"); 
					break;
				} else if(bytes_read < 0) { 
					if (csz==size) eraise("c_triv_ssocket.read() timeout \n");
					break;
				} else {// 	bytes_read>0
					if (foptions & eopt_ReadSize_MAX) break;
					//TODO:
				};
			};
			return size-csz;
	}; 
	virtual int settimeout( uint to_type , uint ms ){
		// SO_RCVTIMEO , SO_SNDTIMEO
		int t = to_type == eop_WRITE ? SO_SNDTIMEO :  to_type ==  eop_READ ? SO_RCVTIMEO : -1;
		if (t<0) {
			eraise("c_triv_ssocket.settimeout() not supported type: %d \n", to_type ); return -1;
		};
		return set_socket_timeout( sock, t , ms );
	}; 
	virtual bool is_open(){
		return (CheckSockRes(sock) && isconnection);
	};

	virtual bool AbortOnClose( bool abort) {
		struct linger ls={ (u_short)(abort?1:0) ,0};
		int res=setsockopt( sock , SOL_SOCKET , SO_LINGER , (char*)&ls , sizeof(ls));
		if (res==SOCKET_ERROR) eraise("Client: error on setsockopt( SO_LINGER ,true, 0 )");
		return res!=SOCKET_ERROR;
	};
	virtual uint setoptions(uint _opt) { //foptions=_opt; return foptions;
		uint oldopt=foptions;
		c_ssocket::setoptions(_opt);
		if ( (oldopt ^ _opt ) & eopt_AbortOnClose )
			AbortOnClose( (_opt & eopt_AbortOnClose) !=0  );
		return foptions;
	}


}; // struct c_triv_ssocket: public c_ssocket

c_ssocket* c_ssocket::newsocket(){
	return new c_triv_ssocket();
};

}; // namespace crsys

