#define _CRT_SECURE_NO_WARNINGS
#include <ctime>
#include <iostream>
#include <iomanip>
#include <tsys/systime.h>
#include <tsys/tb_sysfun.h>
#include "test_state.h"

using stringA = tbgeneral::stringA;
using test_state = tbgeneral::test_ns::test_state;

namespace crsys{


static void test_any() {
	/*
	//std::cout << get_mktime_shift_sec() << std::endl;
	//std::tm t2;
	//std::time_t tt;
	auto tstr = "2022-01-01 18:44:10 +03:00";
	//auto tstr = "1970-01-02 00:00:00 +03:00";
	crsys::msunixtime_t ut;
	auto res = str2msunixtime(ut, tstr);
	ut = ut / 1000;
	std::cout << res << "  " << ut << std::endl;
	std::cout << "msunixtime2string:" << msunixtime2string(ut * 1000).c_str() << std::endl;
	//std::put_time(std::gmtime(&ut), "%c")
	std::cout << std::put_time(std::gmtime(&ut), "%c") << std::endl;
	std::cout << std::put_time(std::gmtime(&ut), "%c %Z") << std::endl;
	std::cout << std::put_time(std::localtime(&ut), "%c %Z") << std::endl;

	//std::strftime((char*)mbstr, sizeof(mbstr), "%A %c", &t2);
	//std::cout << t2;
	//std::cout << "Time:" << mbstr << " == " << t2 << "\n";
	*/
}

static int test_formattime(test_state& err) {
	char buff[256];
	stringA rs;
	crsys::SYSTEMTIME tms[] = { //uint16 wYear, wMonth, wDayOfWeek, wDay, wHour, wMinute, wSecond, wMilliseconds;
		 {2022,8,30, 18,12,22,444 , 5}
		,{2002,1,25, 16,21,32,444 , 5} 
	};
	crsys::format_datetime_def_HL(buff, sizeof(buff), tms[0]);
	if (strcmp(buff, "2022.08.30 18:12:22")) return err.pushf("line:(%d) test_formattime cmp error", __LINE__);
	crsys::format_datetime_def(buff, sizeof(buff), tms[1]);
	if (strcmp(buff, "25.01.2002 16:21:32")) return err.pushf("line:(%d) test_formattime cmp error", __LINE__);
	rs = format_datetime("YYYY.MM.DD hh:mm:ss:cccc", tms[1]);
	if (strcmp(rs, "2002.01.25 16:21:32:0444")) return err.pushf("line:(%d) format_datetime cmp error", __LINE__);
	rs = format_datetime("YY.MM.DD hh:mm:ss:cc", tms[0]);
	if (strcmp(rs, "22.08.30 18:12:22:44")) return err.pushf("line:(%d) format_datetime cmp error", __LINE__);
	msunixtime_t utl,utl1;
	stringA sut;
	str2msunixtime(utl, "1970.01.02 00:00:00");
	if (utl!= (24*60*60*1000) ) return err.pushf("line:(%d) str2msunixtime error", __LINE__);
	str2msunixtime(utl, "2022.05.05 10:22:22");
	sut = msunixtime2string(utl);
	if (strcmp(sut, "2022-05-05 10:22:22")) return err.pushf("line:(%d) msunixtime2string error", __LINE__);
	//auto st1 = unixtime2systime(utl/1000);
	str2msunixtime(utl1, "2022.05.05 10:23:22");
	if ((utl1- utl) != ( 60 * 1000)) return err.pushf("line:(%d) str2msunixtime error", __LINE__);
	str2msunixtime(utl1, "2022.06.02 10:22:22");
	if ((utl1 - utl)/1000 != (28 *24*60*60)) return err.pushf("line:(%d) str2msunixtime error", __LINE__);

	return 0;
}


int cmp(const SYSTEMTIME & l, const SYSTEMTIME& r){
	SYSTEMTIME a[2]={l,r};
	a[0].wDayOfWeek=0; a[1].wDayOfWeek = 0;
	return memcmp( &a[0],&a[1], sizeof(SYSTEMTIME) );
}

static int test_cdatetime(test_state& err) {
	SYSTEMTIME tms[] = { //uint16 wYear, wMonth, wDayOfWeek, wDay, wHour, wMinute, wSecond, wMilliseconds;
		 {2022,8,30, 18,12,22,444}
		,{2002,1,25, 16,21,32,444}
		,{2022,6,8, 12,21,30,555}
	};
	auto uts = cdatetime::unixtimestart;
	size_t i=0;
	for (auto st:tms){
		auto chk1 = cdatetime(st).toSYSTEMTIME();
		if (cmp(st,chk1)) return err.pushf("line:(%d,%d) systime->double->systime", i,__LINE__);
		chk1 = cdatetime::as_msunixtime( cdatetime(st).to_msunixtime() ).toSYSTEMTIME();
		if (cmp(st, chk1)) return err.pushf("line:(%d,%d) systime->unixtime->double->systime", i, __LINE__);
		i++;
	}

	return 0;
}


static int test_realtime(test_state& err){
	uint32_t gtc[] = { 0,0 };
	gtc[0]=crsys::getTickCount();
	auto er1=true;
	for (int i=0;i<10;i++){
		crsys::Sleep(2);
		gtc[1] = crsys::getTickCount();
		if (gtc[1] != gtc[0]) break;
	}
	if (gtc[1]<= gtc[0] || (gtc[1] - gtc[0])>100) return err.pushf("line:(%d) getTickCount error (r-l=%d)", __LINE__, gtc[1] - gtc[0] ) ;
	//std::this_thread::sleep_for(2000ms);
	// 

	return 0;
};
};//crsys

namespace tbgeneral {namespace test_ns {

mDeclareTestProc(test_time,esr_TestIsReady){
//int test_time() {
	
	if (call_test(crsys::test_formattime, "test_formattime")) return 1;
	if (call_test(crsys::test_realtime, "test_realtime")) return 1;
	if (call_test(crsys::test_cdatetime, "test_cdatetime")) return 1;
	
	//crsys::test_any();
	return 0;
};

}};