#pragma once
#ifndef _SYSTIME_H_
#define _SYSTIME_H_

#include <tb_basetypes.h>
//#include <iostream>
//#include <string>
#include "gdata/t_string.h"

namespace crsys{

//using stringA = tbgeneral::stringA;
//using param_stringA=tbgeneral::param_stringA;

typedef uint time32sec;        // измерение в секундах 
typedef uint time32milisec;    // измерение в милисекундах 
typedef uint32_t unixtime_t;   // секунд после 1970.01.01 0:0:0
typedef int64_t msunixtime_t;  // милисекунд после 1970.01.01 0:0:0

struct SYSTEMTIME {
	uint16 wYear, wMonth, wDay, wHour, wMinute, wSecond, wMilliseconds , wDayOfWeek ;
};

// cdatetime: whole part is time in second form 0000 year 
struct cdatetime { 

	enum {
		YEAR_0000 = 0,
		YEAR_1601 = 0, //The FILETIME structure is a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC).
		YEAR_1900 = 693594, // self
		YEAR_1970 = 25569 + YEAR_1900 //time_t Return the time as seconds elapsed since midnight, January 1, 1970. There is no error return. 
	};
	enum enum_timeformat { eformat_win_FILETIME = 1, eformat_time_t = 2 };
	enum { HMSperVal = 1 };
	enum { HMS = 24 * 60 * 60, BASEDYEAR_D = YEAR_0000 };
	//static const float DayPerValue=1.0/HMS;
	static cdatetime unixtimestart; // for unixtime==0 ( from 01.01.1970 )
	double data;


	cdatetime() { data = 0; };
	cdatetime(double d) { data = d; };
	cdatetime(const crsys::SYSTEMTIME& d);
	cdatetime(int year, int month, int day);
	//operator double() const { return data; }
	//operator double& () { return data; }
	//operator crsys::SYSTEMTIME() { return convert(); } ;
	double getdate() { return double(uint64(data / (HMS))); };
	double gettime() { return data - getdate() * HMS; };
	SYSTEMTIME toSYSTEMTIME() const;
	SYSTEMTIME convert() const { return toSYSTEMTIME(); };
	static cdatetime convert(uint64 v, enum_timeformat f);
	msunixtime_t to_msunixtime() const {
		auto v = data - cdatetime::unixtimestart.data;
		return msunixtime_t(v * 1000);
	}
	unixtime_t to_unixtime()const { return unixtime_t(to_msunixtime()/1000);	}


	static cdatetime localshift();
	static cdatetime as_msunixtime(msunixtime_t v) { return cdatetime( cdatetime::unixtimestart.data+double(v)/1000); };
	static cdatetime as_unixtime(unixtime_t v) { return cdatetime(cdatetime::unixtimestart.data + double(v)); };
	static cdatetime now();
	static cdatetime hms(uint h, uint m, uint s, uint ms = 0);
};

inline bool operator==(const cdatetime& l, const cdatetime& r) { return l.data == r.data; }
inline bool operator!=(const cdatetime& l, const cdatetime& r) { return l.data != r.data; }
inline bool operator<(const cdatetime& l, const cdatetime& r) { return l.data < r.data; }
inline bool operator>(const cdatetime& l, const cdatetime& r) { return l.data > r.data; }
inline bool operator<=(const cdatetime& l, const cdatetime& r) { return l.data <= r.data; }
inline bool operator>=(const cdatetime& l, const cdatetime& r) { return l.data >= r.data; }




void GetLocalTime(SYSTEMTIME* LocalTime);
inline SYSTEMTIME GetLocalTime(){ SYSTEMTIME t; GetLocalTime(&t); return t; };

struct ns_timef{
	static const char * frmtYear4; //"%02d.%02d.%04d %02d:%02d:%02d";
	static const char* frmtYear2; //"%02d.%02d.%02d %02d:%02d:%02d";
};


time32_t getTickCount();
unixtime_t getunixtime();
msunixtime_t getunixtime_ms();

inline msunixtime_t to_msunixtime_t(unixtime_t ut){ return msunixtime_t(ut)*1000; }
inline unixtime_t to_unixtime_t(msunixtime_t ut) { return static_cast<unixtime_t>( ut/1000); }

inline void convert(SYSTEMTIME& st, msunixtime_t v) { st = cdatetime::as_msunixtime(v).toSYSTEMTIME(); };
inline void convert(SYSTEMTIME & st , unixtime_t v) { convert(st, to_msunixtime_t(v)  );  } ;
inline void convert(SYSTEMTIME& st, const cdatetime& dt ) { st= dt.toSYSTEMTIME(); };

inline void convert(msunixtime_t& ut , const SYSTEMTIME& st) { ut = cdatetime(st).to_msunixtime();  }
inline void convert(msunixtime_t& ut, unixtime_t v) { ut= to_msunixtime_t(v); };
inline void convert(msunixtime_t& ut, const cdatetime& dt) { ut= dt.to_msunixtime(); };

inline void convert(unixtime_t& ut, const SYSTEMTIME& st) { msunixtime_t mut;convert(mut,st); ut = unixtime_t(mut); };
inline void convert(unixtime_t& ut, msunixtime_t v) {  ut = unixtime_t(v); };
inline void convert(unixtime_t& ut, const cdatetime& dt) { msunixtime_t mut; convert(mut, dt); ut = unixtime_t(mut); };;

inline void convert(cdatetime& dt, const SYSTEMTIME& st) { dt = cdatetime(st); };
inline void convert(cdatetime& dt, msunixtime_t v){ dt= cdatetime::as_msunixtime(v); };
inline void convert(cdatetime& dt, unixtime_t v){ convert(dt, to_msunixtime_t(v)); };


//inline SYSTEMTIME to_systemtime(unixtime_t v) { SYSTEMTIME r; convert(r, v); return r;};
inline SYSTEMTIME to_systemtime(msunixtime_t v){ return cdatetime::as_msunixtime(v).toSYSTEMTIME();};

int format_datetime_to( std::basic_ostream<char>& res, const tbgeneral::param_stringA& frmtstr, msunixtime_t dt);
int format_datetime_to(tbgeneral::stringA& res, const tbgeneral::param_stringA& frmtstr, unixtime_t dt);
int format_datetime_to(tbgeneral::stringA& res, const tbgeneral::param_stringA& frmtstr, msunixtime_t dt);
int format_datetime_to(tbgeneral::stringA& res, const tbgeneral::param_stringA& frmtstr, const SYSTEMTIME& dt);
tbgeneral::stringA format_datetime(const tbgeneral::param_stringA& frmtstr, const SYSTEMTIME& dt);
int format_datetime(const char* frmtstr, const char* buffer, uint bsz, const SYSTEMTIME& dt);
int format_datetime_def(char* buffer, uint bsz, const SYSTEMTIME& dt, const char* frmtstr = 0);
int format_datetime_def_HL(char* buffer, uint bsz, const SYSTEMTIME& dt, const char* frmtstr = 0);


int str2msunixtime(msunixtime_t& res, const char* str, const char* end);
int str2msunixtime(crsys::msunixtime_t& res, const tbgeneral::stringA& v);
//int str2msunixtime(crsys::msunixtime_t& res, const tbgeneral::partstringA & v );
tbgeneral::stringA msunixtime2string(msunixtime_t res);






}; // namespace crsys

#endif // _SYSTIME_H_
