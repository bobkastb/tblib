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
		enum eTestProcStatus { esr_TestNeedUpdate = 0xFF00FF00, esr_TestIsAStub = 0xFF00FF01, esr_TestIsReady = 0 };

		struct test_state {
			std::string res;
			std::string head;

			int push_(const std::string einf);
			int pushe(const std::string einf);
			int pushe(const char* einf, int ecod);
			int pushl(size_t lineno, const char* einf, ...);
			int pushf(const char* einf, ...);
			void write_tostrm(std::ostream& out) const;
		};
		inline std::ostream& operator << (std::ostream& out, const test_state& ts) { ts.write_tostrm(out); return out; }

		typedef int (*testf_type)(test_state& err);
		typedef int (*test_proc_type)();
		int call_test_base(testf_type f, const char* pret = 0);

		struct tTestProcRegistrator {
			test_proc_type proc;
			const char* name;
			eTestProcStatus state;
			int gindex;
			tTestProcRegistrator(test_proc_type proc, const char* name, eTestProcStatus st);
			int call_test();
		};
		void RegisterTestProc(tTestProcRegistrator* preg);

		struct tTestCollection {
			using preg_t = tTestProcRegistrator*;
			using name_type = std::string;
			static tTestCollection& gettests();
			std::vector<preg_t> regslist;
			std::map<name_type, preg_t> index_nms;
			void Register(tTestProcRegistrator* preg);
			preg_t getbyname(const std::string& nm);
			static int call_all_tests(bool stoponerr);
			static int call_single_tests(const char * name);
		};


	}
};

#define mRegisterTestProc( proc ,state ) tTestProcRegistrator reg_##proc(proc,#proc,state)
#define mDeclareTestProc( proc ,state ) int proc(); tTestProcRegistrator reg_##proc(proc,#proc,state); int proc()
