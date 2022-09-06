#include <stdarg.h>
#include <stdio.h>


int tb_snprintf( char * buffer, size_t n, const char * format, ... ){
	va_list args;
	va_start (args, format);
	return vsnprintf (buffer,n,format, args);
};
int tb_vsnprintf (char * buffer, size_t n, const char * format, va_list args ){
	return vsnprintf (buffer,n,format, args);
};
