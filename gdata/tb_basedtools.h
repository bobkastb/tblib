#pragma once

#include "tb_basetypes.h"
#include <stdarg.h>
#include <string.h>
#include <wchar.h>

namespace tbgeneral{ 

#define ZEROMEM( obj ) memset(&(obj),0,sizeof(obj))


// for g++ warning -Wclass-memaccess
void memreset( void* data , size_t size);
void memcopy_nwarn( void* dist , const void* src , size_t size);
void memmove_nwarn( void* dist , const void* src , size_t size);
template <class Obj_t> void RESETMEM( Obj_t & obj ) { memreset(&obj , sizeof(obj)); }


template <class T>  void ResetClassValue(T& v) { v=T();}

int bitpower(uint l);

inline size_t strlen_s(const char * d) {  return (d? ::strlen(d) : 0  ); }
inline size_t strlen_s(const wchar * d) {   return d ? wcslen(d) : 0; };

int strcmp( const char * l , const char * r , size_t lsz , size_t rsz );
int strcmp( const wchar_t * l , const wchar_t * r , size_t lsz , size_t rsz );
int f_stricmp(const wchar_t * l , const wchar_t * r , size_t lsz , size_t rsz);
int f_stricmp(const char * l , const char * r , size_t lsz , size_t rsz);
void f_strtoupper( char * l , size_t sz); 
void f_strtolower( char * l , size_t sz); 
void f_strtoupper( wchar_t * l , size_t sz);
void f_strtolower( wchar_t * l , size_t sz);


template <class charT> int stricmp( const charT * l, const charT * r  ) { return f_stricmp( l , r , strlen_s( l ) , strlen_s( r ) ); }

bool isspaceChar( char c );
bool isspaceChar( wchar_t c );



template <typename TControl> struct va_params_t_any{
	TControl format;
	va_list vars;
	bool vars_present;
	va_params_t_any( const TControl & _format ) {memreset(this,sizeof(*this)); format=_format;}
	va_params_t_any( ) { memreset(this,sizeof(*this));  }
};
using va_params_t = va_params_t_any<const char*>;
template <typename TControl> va_params_t_any<TControl> make_va_params_t(const TControl & _f){
	va_params_t_any<TControl> r; r.format = _f; return r;
}
#define va_Init( name , vacontrol ) auto name = tbgeneral::make_va_params_t(vacontrol); name.format=vacontrol; name.vars_present=true; va_start(name.vars,vacontrol);
//Example : void testformat(int options, const char* format,...){ va_Init(vaparams,format);	server_format( options,vaparams); ...


}; // namespace tbgeneral{ 
