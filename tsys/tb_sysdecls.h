#pragma once

#ifndef _WINDOWS
//_WINDOWS_
#if defined(_WIN32) || defined(_WINDOWS_)
#define _WINDOWS
#endif
#endif

#ifndef _WINDOWS
#define _LINUX	
#endif

#ifdef _MSC_VER
#define warning_MSVC( cmd , id ) __pragma(warning(cmd:id))
#define warning_PUSH() __pragma(warning(push))
#define warning_POP() __pragma(warning(pop))
#elif defined(__GNUC__)
#define warning_PUSH() _Pragma("GCC diagnostic push")
#define warning_POP() _Pragma("GCC diagnostic pop")
#define warning_MSVC( cmd , id )
#else 
#define warning_MSVC( cmd , id )
#endif



#ifdef _MSC_VER
	#define _thread_var __declspec(thread)
#elif defined(__GNUC__)
	#define _thread_var __thread
#endif


#if defined(_LINUX)

#define OS_TYPE_NAME "linux"
#define EXE_EXTENTION ""
#define MODULE_EXPORT
#define MODULEEXPORTCALL 
		// _cdecl
#define SYSAPICALL 
#define STDLIBCALL
#define DEFAULT_DLL_EXT ".so"
//#define stricmp strcasecmp
#define __cdecl



#elif defined(_WINDOWS) // ============= WINDOWS ============

#define OS_TYPE_NAME "win32"
#define EXE_EXTENTION ".exe"
#define DEFAULT_DLL_EXT ".dll"

//#define SYSAPICALL __stdcall
#define SYSAPICALL __stdcall
#define MODULEEXPORTCALL _cdecl
	// _cdecl только потому что __declspec(dllexport) __stdcall - декорирует имена
#define STDLIBCALL __stdcall
#define MODULE_EXPORT __declspec(dllexport)

//int __stdcall gettimeofday(struct timeval* tv, void* timezone=0);



#endif
