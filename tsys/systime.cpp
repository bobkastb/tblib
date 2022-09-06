#define _CROSS_SYS_COD

//#include <tsys/precompiled.h>
#include <sys/timeb.h>
#include <time.h>

#include "cross_sys.h"
#include "tb_sysfun.h"
#include "gdata/t_string.h"
#include "systime.h"

#include "gdata/tb_algo.h"
#include "gdata/tb_c_tools.h"

warning_MSVC(disable , 4996)
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#endif



#ifdef _WINDOWS

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef SYSTEMTIME w_SYSTEMTIME;
#ifndef _WINSOCKAPI_
	struct timeval {  long tv_sec;  long tv_usec;};
#endif

#else

#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>
#endif

#define EPOCH_DIFF 11644473600LL

/**
 * number of seconds from 1 Jan. 1601 00:00 to 1 Jan 1970 00:00 UTC
 */

#include <ctime>

using stringA = tbgeneral::stringA;
using param_stringA=tbgeneral::param_stringA;


namespace crsys{



msunixtime_t getunixtime_ms(){
	std::time_t t = std::time(0);
	std::localtime(&t);
	return msunixtime_t( (unixtime_t)t )*1000;
};
unixtime_t getunixtime(){
	std::time_t t = std::time(0);
	std::localtime(&t);
	return (unixtime_t)t;
}



static int64_t get_mktime_shift_sec() {
	std::tm tm0 = { 0 };
	tm0.tm_isdst = -1;
	tm0.tm_mday = 2;
	tm0.tm_year = 70;
	auto tD = mktime(&tm0);
	return tD - 24 * 3600;
}

static int _scansignonly(const char* s) {
	for (; *s && *s == ' '; s++) {}
	int res = *s == '+' ? 1 : *s == '-' ? -1 : 0;
	if (!res) return res;
	s++;
	for (; *s && *s == ' '; s++) {}
	return  *s ? 0 : res;
}

int str2msunixtime(crsys::msunixtime_t& res, const char* str, const char* end) {
	char strBuffer[512];
	if (end == 0) end = str + strlen(str);
	std::tm tm0 = { 0 };
	int zone[2] = { 0 };
	char s_tmp[256];
	tbgeneral::stringA sA;
	sA.assign_storage(strBuffer,sizeof(strBuffer));
	sA.assign(str, end);
	sA.append("+00:00");
	if (sA.size() > 256) return -1;
	//sscanf_s()
	auto cc = std::sscanf(sA.c_str(), "%d%*[-/. ]%d%*[-/. ]%d%*[T ]%d:%d:%d%[-+ ]%d:%d", &tm0.tm_year, &tm0.tm_mon, &tm0.tm_mday, &tm0.tm_hour, &tm0.tm_min, &tm0.tm_sec, s_tmp, &zone[0], &zone[1]);
	if (cc < 8) return -1;
	if (zone[0] > 12 || zone[1] > 59) return -2;
	auto signZ = _scansignonly(s_tmp);
	if (signZ == 0) return -3;
	tm0.tm_isdst = -1;
	tm0.tm_year -= 1900;
	tm0.tm_mon -= 1;
	auto tD = mktime(&tm0); 
	if (tD < 0) return -1;
	tD -= get_mktime_shift_sec();
	tD -= signZ * (zone[0] * 60 + zone[1]) * 60;
	res = tD * 1000;
	return 0;
}

int str2msunixtime(crsys::msunixtime_t& res, const tbgeneral::stringA & v) {
	return str2msunixtime(res, v.data(), v.end());
};

tbgeneral::stringA msunixtime2string(crsys::msunixtime_t res) {
	char buff[256];
	res = res / 1000;
	auto tmp = std::gmtime(&res);
	std::strftime(buff, sizeof(buff), "%F %T", tmp);
	return buff;
}


#ifdef _WINDOWS

void GetLocalTime(SYSTEMTIME* lt){
	w_SYSTEMTIME slt;
	::GetLocalTime(&slt);
	lt->wYear = slt.wYear;
	lt->wMonth = slt.wMonth;
	lt->wDay =slt.wDay;
	lt->wDayOfWeek = slt.wDayOfWeek;
	lt->wHour = slt.wHour;
	lt->wMinute = slt.wMinute;
	lt->wSecond = slt.wSecond;
	lt->wMilliseconds = slt.wMilliseconds;
};

#else

void GetLocalTime(SYSTEMTIME* LocalTime) {
	timeval tvs;
	gettimeofday(&tvs, 0);
	tm* tml=localtime(&tvs.tv_sec);
	LocalTime->wYear=1900+tml->tm_year;
	LocalTime->wMonth=tml->tm_mon+1;
	LocalTime->wDayOfWeek=tml->tm_wday;
	LocalTime->wDay=tml->tm_mday;
	LocalTime->wHour=tml->tm_hour;
	LocalTime->wMinute=tml->tm_min;
	LocalTime->wSecond=tml->tm_sec;
	LocalTime->wMilliseconds=tvs.tv_usec/1000;
}

#endif


static UINT64 getfiletime(const timeval& tv) {
	UINT64 result=((UINT64)EPOCH_DIFF+tv.tv_sec)*10000000LL+(tv.tv_usec*10);
	return result;
}

static timeval gettimeval(UINT64 filetime) {
	timeval tv;
	tv.tv_sec=(long)(filetime/10000000LL-EPOCH_DIFF);
	tv.tv_usec=filetime%10000000LL;
	return tv;
}

static void getsystime(const timeval& tv, SYSTEMTIME& st) {
#ifdef _WIN32
	tm* tml=_localtime32(&tv.tv_sec);
#else
	tm* tml=localtime(&tv.tv_sec);
#endif

	st.wYear=1900+tml->tm_year;
	st.wMonth=tml->tm_mon+1;
	st.wDayOfWeek=tml->tm_wday;
	st.wDay=tml->tm_mday;
	st.wHour=tml->tm_hour;
	st.wMinute=tml->tm_min;
	st.wSecond=tml->tm_sec;
	st.wMilliseconds=uint16(tv.tv_usec/1000);
}
static timeval gettimeval(const SYSTEMTIME& st) {
	timeval tv;
	tm tml;

	tml.tm_year=st.wYear-1900;
	tml.tm_mon=st.wMonth-1;
	tml.tm_wday=st.wDayOfWeek;
	tml.tm_mday=st.wDay;
	tml.tm_hour=st.wHour;
	tml.tm_min=st.wMinute;
	tml.tm_sec=st.wSecond;
	tv.tv_usec=(long)st.wMilliseconds*1000;

#ifdef _WIN32
	tv.tv_sec=_mktime32(&tml);
#else
	tv.tv_sec=mktime(&tml);
#endif

	return tv;
}

/*
bool operator==(const SYSTEMTIME& a, const SYSTEMTIME& b) {
	return !memcmp(&a, &b, sizeof(a));
}

bool operator!=(const SYSTEMTIME& a, const SYSTEMTIME& b) {
	return !(a==b);
}

bool operator<(const SYSTEMTIME& a, const SYSTEMTIME& b) {
	if (a.wYear!=b.wYear) return (a.wYear<b.wYear);
	if (a.wMonth!=b.wMonth) return (a.wMonth<b.wMonth);
	if (a.wDay!=b.wDay) return (a.wDay<b.wDay);
	if (a.wHour!=b.wHour) return (a.wHour<b.wHour);
	if (a.wMinute!=b.wMinute) return (a.wMinute<b.wMinute);
	if (a.wSecond!=b.wSecond) return (a.wSecond<b.wSecond);
	if (a.wMilliseconds!=b.wMilliseconds) return (a.wMilliseconds<b.wMilliseconds);
	return true;
}


INT64 operator-(const SYSTEMTIME& a, const SYSTEMTIME& b) { //разница во времени в миЛлиСЕКУНДАХ!
	timeval tva(gettimeval(a)), tvb(gettimeval(b));
	INT64 result=(tva.tv_sec-tvb.tv_sec)*1000+(tva.tv_usec-tvb.tv_usec)/1000;
	return result;
}

SYSTEMTIME operator-(const SYSTEMTIME& a, DWORD sec) {
	SYSTEMTIME st; timeval tva(gettimeval(a));
	tva.tv_sec-=sec;
	getsystime(tva, st);
	return st;
}

SYSTEMTIME operator+(const SYSTEMTIME& a, DWORD sec) {
	SYSTEMTIME st; timeval tva=gettimeval(a);
	tva.tv_sec+=sec;
	getsystime(tva, st);
	return st;
}
*/
int format_datetime_def_HL(char * buffer,uint bsz, const crsys::SYSTEMTIME & dt , const char* frmtstr ){
    return tb_snprintf(buffer,bsz, frmtstr ? frmtstr : "%04d.%02d.%02d %02d:%02d:%02d" ,dt.wYear,dt.wMonth,dt.wDay , dt.wHour,dt.wMinute,dt.wSecond );
};

int format_datetime_def(char * buffer,uint bsz, const crsys::SYSTEMTIME & dt, const char* frmtstr  ){
	 std::string res; 
    return tb_snprintf(buffer,bsz, frmtstr ? frmtstr : "%02d.%02d.%04d %02d:%02d:%02d" ,dt.wDay , dt.wMonth, dt.wYear , dt.wHour,dt.wMinute,dt.wSecond );
};
int format_datetime(const char* frmtstr, const char* buffer,uint bsz, const crsys::SYSTEMTIME & dt){
	//TODO:
	return 0;
};

int format_datetime_to(std::basic_ostream<char>& os, const tbgeneral::param_stringA& frmtstr, msunixtime_t dt) {
	char buffer[128]; stringA str;
	str.assign_storage(buffer);
	format_datetime_to(str, frmtstr, dt);
	os << str;
	return 0;
};

int format_datetime_to(tbgeneral::stringA& res, const tbgeneral::param_stringA& frmtstr, unixtime_t dt){
	return format_datetime_to(res, frmtstr, to_msunixtime_t(dt) );
};

int format_datetime_to(stringA& res, const param_stringA& frmtstr, msunixtime_t dt){
	return format_datetime_to(res, frmtstr , to_systemtime(dt) );
};

const char* ns_timef::frmtYear4 = "YYYY.MM.DD hh:mm:ss";
const char* ns_timef::frmtYear2 = "YY.MM.DD hh:mm:ss";

int format_datetime_to(stringA& res, const param_stringA& frmtstr, const SYSTEMTIME& dt){
	auto capanew = res.size() + frmtstr.param.size();
	if (res.capacity()< capanew) res.reserve(capanew);
	auto s = frmtstr.param.cbegin() , e= frmtstr.param.cend();
	char buffer[128];
	for (; s<e; s++) {
		uint val, sz;
		switch (s[0]) {
		case 'Y': val = dt.wYear; break;
		case 'M': val = dt.wMonth; break;
		case 'D': val = dt.wDay; break;
		case 'h': val = dt.wHour; break;
		case 'm': val = dt.wMinute; break;
		case 's': val = dt.wSecond; break;
		case 'c': val = dt.wMilliseconds; break;
		default: res.push_back(*s); continue;
		};
		char c = *s;
		for (sz = 0; c == *s; s++, sz++);
		s--; sz = tbgeneral::min<uint>(sz, 4);
		sprintf(buffer, "%08d", val);
		res.append(buffer + 8 - sz, sz);
	};
	return 0;
};


stringA format_datetime(const param_stringA& frmt, const crsys::SYSTEMTIME & dt){
	stringA res;
	format_datetime_to( res, frmt , dt );
	return res;
};

}; // namespace crsys




namespace crsys {

	// Days between 1/1/0001 and 12/31/1899 
//#define __DateDelta 693594

// Days between TDateTime basis (12/31/1899) and Unix time_t basis (1/1/1970) }

//#define __UnixDateDelta = 25569;


struct divmod
{ 
   uint d,re; 
   divmod(){ };
   divmod( uint ac ,uint ad) { d= ac/ad; re=ac % ad; }
};
/*
struct datetimestump
{
    int ymd,t;
	datetimestump(){};
	datetimestump( double d ) { ymd = d; t = (d-ymd)*MSecsPerDay; }; 
	datetimestump( int aymd , int at) { ymd=aymd; t=at; };
	operator double( ) { return ymd + (double(t))/MSecsPerDay; }; 
};
*/

enum{  D1 = 365 ,  D4 = D1 * 4 + 1 ,  D100 = D4 * 25 - 1 ,  D400 = D100 * 4 + 1};

struct TDayRec{ int cnt , startinY; };
static  int DayCntTable[2][12] = 
{{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}, {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};
static TDayRec DayFullTable[2][12]={0};
static void prepare_DayFullTable()
{    if (DayFullTable[0][0].cnt) return; 
     for (int y=0;y<2;y++)
       for (int m=0,c=0;m<12;m++)
	   {  DayFullTable[y][m].cnt = DayCntTable[y][m];
	      DayFullTable[y][m].startinY=c; c+=DayCntTable[y][m]; 
	   };  
};
static int IsBigYear( int year )
{ 
	return ( (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0)) ? 1 : 0);
}


static cdatetime encodetime( int h , int m , int s , int ms ) 
{   double hms = double(h*60*60 + m*60 + s) + double(ms)/1000;
	return double( hms );
}

static void decodetime( double dt, crsys::SYSTEMTIME & dinf ) 
{ uint hms = uint( cdatetime(dt).gettime()*1000 );
 // int h,m,s,ms;
  divmod dm=divmod( hms , 1000 ); dinf.wMilliseconds = dm.re; 
  dm=divmod( dm.d , 60 ); dinf.wSecond = dm.re;
  dm=divmod( dm.d , 60 ); dinf.wMinute = dm.re;
  dm=divmod( dm.d , 24 ); dinf.wHour = dm.re;
  if (dm.d) RAISE(" bad tdate time format at decode time ");
}

static void decodedate( double dt, crsys::SYSTEMTIME & dinf , uint baseyear_days = cdatetime::BASEDYEAR_D ); 
static cdatetime encodedate(  int ay , int am , int ad , uint baseyear_days=cdatetime::BASEDYEAR_D ); 

static cdatetime encodedate(  int ay , int am , int ad , uint baseyear_days ) 
{  uint64 ymd;
   int y,d;
   prepare_DayFullTable();
   am = am>1 ? am-1 : 0; ad = ad>1 ? ad-1 : 0; 
   //d = (DayFullTable[IsBigYear(ay)][am-1]).startinY;
   d = (DayFullTable[IsBigYear(ay)][am]).startinY;
   y= ay>1 ? ay-1 : 0;
   ymd=0;
   ymd=y * 365 + y / 4 - y / 100 + y / 400 + d + ad - baseyear_days;
   return double(ymd*cdatetime::HMS);
}

static void decodedate( double dt, crsys::SYSTEMTIME & dinf , uint baseyear_days ) 
{   int y,m,d; int bigy;
    prepare_DayFullTable();
	uint32 ymd =uint32( cdatetime(dt).getdate()); ymd += baseyear_days;
	dinf.wDayOfWeek = (ymd % 7) + 1;
	//divmod o400( ymd - 1 , D400 ); 
	divmod o400( ymd  , D400 ); 
	divmod o100( o400.re  , D100 ); if (o100.d==4) {o100.d--; o100.re+=D100;};
	divmod o4( o100.re  , D4 );
	divmod oD( o4.re  , D1 );       if (oD.d==4) {oD.d--; oD.re+=D1;};
	y = o400.d*400+o100.d*100+o4.d*4+oD.d+1;
	d = oD.re;
	m = tbgeneral::min(11,d / 30);
	bigy = IsBigYear(y);
	if ( d>=DayFullTable[bigy][m].startinY ) 
	{  if ((m<11)&&( d>=DayFullTable[bigy][m+1].startinY )) m++;}
	else { m--; } 

	dinf.wDay  =d- DayFullTable[bigy][m].startinY + 1;
	dinf.wMonth =m + 1;
	dinf.wYear =y ; 
};


cdatetime cdatetime::unixtimestart= cdatetime({1970,1,1, 0});
cdatetime::cdatetime(const crsys::SYSTEMTIME & d){
	data = encodedate( d.wYear , d.wMonth , d.wDay  ,BASEDYEAR_D).data;
	data += encodetime( d.wHour , d.wMinute , d.wSecond  , d.wMilliseconds ).data;
};

cdatetime::cdatetime(int year , int month , int day){
	data = encodedate( year , month , day  ,BASEDYEAR_D).data;
};

SYSTEMTIME cdatetime::toSYSTEMTIME() const {
	SYSTEMTIME r;
	decodedate(data, r, BASEDYEAR_D);
	decodetime(data, r);
	return r;
};


cdatetime cdatetime::convert(uint64 v , enum_timeformat f){
	int year; double m=0;
	switch (f) {
		case cdatetime::eformat_win_FILETIME: year = 1601; m= 100E-9; break;
		case cdatetime::eformat_time_t: year = 1970;  m=1;break;
		default: return double(v);		
	};
	cdatetime y= encodedate( year , 1 , 1  , YEAR_0000 );
	y.data+= double(v)*m;
	return y;
};

cdatetime cdatetime::localshift(){
	cdatetime res;
    struct timeb timeptr;
    ftime(    &timeptr );
	res = (double(-timeptr.timezone)*60 + (timeptr.dstflag ? 60*60 : 0))*HMSperVal;
	return res;
};
cdatetime cdatetime::now(){
	cdatetime res;
    struct timeb timeptr;
    ftime(    &timeptr );
	res = convert( timeptr.time , eformat_time_t );
	res.data += double( timeptr.millitm )/1000 + (double(-timeptr.timezone)*60 + (timeptr.dstflag ? 60*60 : 0))*HMSperVal ;
	return res;
};
cdatetime cdatetime::hms(uint h , uint m , uint s , uint ms){
	return  (h*60*60 + m*60 + s + double(ms)/1000)*HMSperVal ;
}; 

}; // namespace tbgeneral 
