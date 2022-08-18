#pragma once

#include <stdarg.h>
#include <stddef.h>

// snprintf - supported only with 2011 y
int tb_snprintf( char * s, size_t n, const char * format, ... );
int tb_vsnprintf (char * s, size_t n, const char * format, va_list arg );
