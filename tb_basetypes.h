#pragma once
#ifndef __TB_BASETYPES
#define __TB_BASETYPES

#include <cstdint>
/*
	ptrs: 	size_t , ptrdiff_t , intptr_t , uintptr_t 
	--
*/



#ifdef _MSC_VER
typedef __int64            _int64_t;
typedef unsigned __int64   _uint64_t;
typedef unsigned long	_DWORD_TYPE_;

#elif defined(__GNUC__)

typedef long long            _int64_t;
typedef unsigned long long   _uint64_t;
typedef unsigned int	_DWORD_TYPE_;

#endif

typedef char			int8;
typedef unsigned char	uint8;
typedef short			int16;
typedef unsigned short	uint16;
typedef int				int32;
typedef unsigned int	uint32;
typedef int64_t			int64;
typedef uint64_t		uint64;

typedef wchar_t wchar;

#ifndef STDINTTYPES_NAMES
#define STDINTTYPES_NAMES
typedef _DWORD_TYPE_	dword,DWORD;
typedef uint16			word,WORD;
typedef uint8			byte,BYTE;
typedef uint64		qword,QWORD;
#endif





typedef _int64_t		INT64;
typedef _uint64_t		UINT64;
typedef void *			LPVOID;
typedef uint32			time32_t;
typedef uint64			time64_t;
typedef unsigned char	u_char;

//typedef unsigned long tt_ulong;
//typedef signed long slong;
typedef unsigned int tt_ulong;
typedef signed int slong;

typedef unsigned int uint;
typedef unsigned int sint;

//#ifndef arraylen
#define arraylen(a) (sizeof(a)/sizeof(a[0]))
//#endif
#define TSIZE(X) (sizeof(X)/sizeof(X[0]))

#define ARRAYLEN( a ) (sizeof(a)/sizeof(a[0]))

#define byte_at(x) (*(uint8*)(x))
#define word_at(x) (*(uint16*)(x))
#define dword_at(x) (*(uint32*)(x))
typedef void (*voidproc)();


/*
typedef unsigned char  byte;          // Unsigned character (byte)
typedef unsigned char  uchar;          // Unsigned character (byte)
typedef unsigned short ushort;         // Unsigned short
typedef unsigned short uint16;         // Unsigned short
typedef unsigned int   uint;           // Unsigned integer
typedef unsigned int   uint32;         // Unsigned short
typedef unsigned long  ulong;          // Unsigned long
typedef unsigned __int64  uint64;          // Unsigned long
typedef signed __int64  int64;          // Unsigned long
*/

#endif
