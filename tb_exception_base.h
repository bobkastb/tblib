#pragma once

#include <exception>
#include <tb_basetypes.h>

#ifdef _DEBUG
#define __FULL_FILENAME_ __FILE__
#else 
#define __FULL_FILENAME_ "..."
#endif




#define RAISE throw crsys::exception( __FULL_FILENAME_ , __LINE__  ).format
#define RAISE_I( code ) throw crsys::iexception( code )
#define panic_error() panic_error_do(0 , __FULL_FILENAME_ , __LINE__  )
void panic_error_do(const char x , const char * _file, int _linen   );

namespace crsys{

void raise_do(const char * x , const char * _file, int _linen   );
void eraise(const char* frmt,...);


struct c_exceptioninfo { const char * file; int linenumber; c_exceptioninfo(){ file=0; linenumber=0; };
		c_exceptioninfo(const char * _file, int _linenumber){ file=_file;linenumber=_linenumber; }; 
};

class iexception : public std::exception{
	public:
	uint ecode;
    iexception() throw() { ecode=0;  };
    iexception(uint c) throw() { ecode=c;  };
};


class exception : public std::exception
  {
  public:
    c_exceptioninfo einf;
    char* ex;
    exception() throw() { ex=0;  };
	exception(const char * file, int linenumber){ ex=0; einf.file=file; einf.linenumber=linenumber; }; 
	exception( const c_exceptioninfo & ei ) { ex=0; einf = ei; };
    exception(const char* w) throw() { ex=0; assign_text(w); };
	~exception() throw()  { assign_text(0); };
	exception & format(const char* frmt,...);
	virtual const char* what() const throw() { return ex; };
	private:  
	void assign_text( const char* s); 
  }; // class exception

}

namespace tbgeneral{
	void bibbf( const char * mess  , ...   ); 
};
