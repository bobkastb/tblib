#pragma once

#include <string>


namespace crsys{
struct c_ssocket{
	enum enum_Operations{ eop_None , eop_Connect , eop_WRITE , eop_READ };
	enum enum_Options{ eopt_None , 
			eopt_ReadSize_MAX=1  // параметр size -задает максимальный размер чтения , иначе - обязательный
			,eopt_AbortOnClose=2
			,eopt_Default=0
	};
	std::string port_c , ip;
	uint foptions;
	virtual ~c_ssocket(){};
	c_ssocket() { foptions = eopt_Default; };
	virtual bool init( const char * _port , const char * ip){ this->ip = ip; this->port_c = _port; return true; };
	virtual bool connect()=0;
	virtual int write( const void * data , uint size)=0; 
	virtual int read( void * data , uint size)=0; 
	virtual int settimeout( uint to_type , uint ms )=0; 
	virtual bool is_open()=0;

	virtual uint getoptions() { return foptions;}
	virtual uint setoptions(uint _opt) { foptions=_opt; return foptions;}
	virtual int native() { return -1;}
	static c_ssocket* newsocket();
};
};
