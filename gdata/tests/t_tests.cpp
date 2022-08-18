
#include "t_tests.h"


namespace tbgeneral {
	namespace test_ns {

		std::string format_ss(const char* format, ...) {
			//std::string buff;
			char buffer[4 * 1024];
			va_Init(vaparams, format);
			vsnprintf(buffer, sizeof(buffer), vaparams.format, vaparams.vars);
			return buffer;
		};
		std::string format_ssf(const char* format, va_params_t& vaparams) {
			//std::string buff;
			char buffer[4 * 1024];
			//va_Init(vaparams, format);
			vsnprintf(buffer, sizeof(buffer), vaparams.format, vaparams.vars);
			return buffer;
		};
		tTestProcRegistrator::tTestProcRegistrator(test_proc_type proc, const char* name, eTestProcStatus st) {
			this->proc = proc;
			this->name = name;
			this->state = st;
			gindex = -1;
			RegisterTestProc(this);
		};
		void RegisterTestProc(tTestProcRegistrator* preg) {
			tTestCollection::gettests().Register(preg);
		};
		void tTestCollection::Register(tTestProcRegistrator* preg) {
			name_type key = preg->name;
			if (getbyname(key) != 0) {
				std::cout << "try repeat registration for: " << key << "\n";
			}
			preg->gindex = regslist.size();
			this->regslist.push_back(preg);
			this->index_nms[key] = preg;

		};
		tTestCollection::preg_t tTestCollection::getbyname(const std::string& nm) {
			auto res = index_nms.find(nm);
			if (res != index_nms.end()) return res->second;
			return 0;
		};
		tTestCollection& tTestCollection::gettests() {
			static tTestCollection tests;
			return tests;
		};

		//typedef int (*testf_type)(test_state& err);
		int call_test_base(testf_type f, const char* pret) {
			test_state ts;
			//auto oldAlObjCnt = tObjMapAlloc::g.size();
			auto v = f(ts);
			//if (oldAlObjCnt != tObjMapAlloc::g.size());	v = ts.pushe("tObjMapAlloc::No all destructor calls!");
			if (!v) return 0;
			if (!pret) pret = "";
			std::cout << "FAIL:" << pret << ":" << ts << "\n";
			return -1;
		}

	int test_state::push_(const std::string einf) { res.insert(0, einf.data(), einf.size()); return 1; }
	int test_state::pushe(const std::string einf) {
		if (head.size()) { push_("[" + head + "]:"); head = ""; }
		push_(einf); return 1;
	}
	int test_state::pushe(const char* einf, int ecod) { pushe(format_ss("%s [%d]", einf, ecod));	return ecod; }
	int test_state::pushl(size_t lineno, const char* einf, ...) {
		va_Init(vaparams, einf);
		auto res = format_ssf(einf, vaparams);
		res = format_ss("%s (line:%d)", res.c_str(), lineno);
		pushe(res);	return static_cast<int>(lineno);
	}
	int test_state::pushf(const char* einf, ...) {
		va_Init(vaparams, einf);
		auto res = format_ssf(einf, vaparams);
		pushe(res);	return 1;
	}
	void test_state::write_tostrm(std::ostream& out) const { 
		out << res; 
	};


	int tTestProcRegistrator::call_test() {
		std::cout << "start test <" << name << "> ...";
		auto r = this->proc();
		if (r) return 1;
		std::cout << "passed" << (this->state == esr_TestNeedUpdate ? " but wait a correction!" : "") << "\n";
		return 0;
	}

	int tTestCollection::call_all_tests(bool stoponerr) {
		int errs = 0;
		std::vector<std::string> stublist;
		auto& tests = tTestCollection::gettests();
		for (auto& tr : tests.regslist) {
			if (tr->state == esr_TestIsAStub) { stublist.push_back(tr->name); continue; }
			auto r = tr->call_test();
			if (stoponerr && r) return 1;
			errs += r ? 1 : 0;
		}
		if (stublist.size()) {
			std::cout << "Stub list[" << stublist.size() << "]: ";
			auto sep = "";
			for (auto& nm : stublist) {
				std::cout << sep << nm;
				sep = " ,";
			}
			std::cout << "\n";
		}
		return errs;
	}

	int tTestCollection::call_single_tests(const char* name){
		auto& tests = tTestCollection::gettests();
		auto t = tests.getbyname( name );
		return t->call_test();
	};



}};//namespace tbgeneral { test_ns {
