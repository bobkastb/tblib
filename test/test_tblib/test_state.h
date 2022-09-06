#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "gdata/tb_basedtools.h"


namespace tbgeneral {
	namespace test_ns {
std::string format_ss(const char* format, ...);
std::string format_ssf(const char* format, va_params_t& vaparams);
enum eTestProcStatus { esr_TestNeedUpdate=0xFF00FF00 , esr_TestIsAStub =0xFF00FF01 , esr_TestIsReady=0 };

struct test_state {
	std::string res;
	std::string head;

	int push_(const std::string einf) { res.insert(0, einf.data(), einf.size()); return 1; }
	int pushe(const std::string einf) {
		if (head.size()) { push_("[" + head + "]:"); head = ""; }
		push_(einf); return 1;
	}
	//int pushe(const char* einf ) { pushe(  einf );	return -1; }
	int pushe(const char* einf, int ecod) { pushe(format_ss("%s [%d]", einf, ecod));	return ecod; }
	int pushl(size_t lineno, const char* einf, ...) {
		va_Init(vaparams, einf);
		auto res = format_ssf(einf, vaparams);
		res = format_ss("%s (line:%d)", res.c_str(), lineno);
		pushe(res);	return static_cast<int>(lineno);
	}
	int pushf(const char* einf, ...) {
		va_Init(vaparams, einf);
		auto res = format_ssf(einf, vaparams);
		pushe(res);	return 1;
	}
	void write_tostrm(std::ostream& out) const { out << res; };
};
inline std::ostream& operator << (std::ostream& out, const test_state& ts) { ts.write_tostrm(out); return out; }

typedef int (*testf_type)(test_state& err);
typedef int (*test_proc_type)();
int call_test(testf_type f, const char* pret = 0);

struct tTestProcRegistrator{
	test_proc_type proc;
	const char* name;
	eTestProcStatus state;
	int gindex;
	tTestProcRegistrator(	test_proc_type proc , const char* name , eTestProcStatus st );
};
void RegisterTestProc(tTestProcRegistrator * preg);

struct tTestCollection{
	using preg_t= tTestProcRegistrator*;
	using name_type= std::string;
	static tTestCollection& gettests();
	std::vector<preg_t> regslist;
	std::map<name_type,preg_t> index_nms;
	void Register(tTestProcRegistrator * preg);
	preg_t getbyname( const std::string & nm);
};


}};
#define mRegisterTestProc( proc ,state ) tTestProcRegistrator reg_##proc(proc,#proc,state)
#define mDeclareTestProc( proc ,state ) int proc(); tTestProcRegistrator reg_##proc(proc,#proc,state); int proc()
