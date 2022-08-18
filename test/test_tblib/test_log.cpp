#include "test_darray.h"
static t_operation_statistic opstat_l(_DBG_ARR_START, _DBG_ARR_END, t_operation_statistic::get_DBG_ARR_names());
#define DEBUG_TECH_ARRAY(opcode,mask) opstat_l.onUseOperation(opcode, mask);


#include "test_state.h"
#include "tsys/systime.h"
#include "tsys/tb_log.h"




namespace tbgeneral { namespace test_ns {

	static int test_log_localb(test_state& err) {

		log_object_t log;
		stringA resl , ress_nohdr; ress_nohdr.reserve(1024); resl.reserve(1024);

		auto redir_NH = [&](log_msg_t* m, const char* d, size_t sz){ 
			ress_nohdr.assign(d, sz); 
		};
		auto redir = [&](log_msg_t* m, const char* d, size_t sz){	resl.assign(d, sz);	};
		//mode_ALWAYSPRINTF = 1, mode_WideChars = 2, mode_DEFAULT = 0, mode_TIME_HL = 4, mode_NoAppend = 8, mode_NoUseLocalBuff = 0x10, mode_NoPushTIME = 0x20
		log.log_open("", "pref", false, log_object_t::mode_TIME_HL | log_object_t::mode_NoCout);
		log.add_redirector(redir);
		log.add_redirector(redir_NH , log_decls_t::efrd_NoHeader);

		int mstst[] = { opstat_l._count_op_alloc , opstat_l._count_op_free };
		log.push(0,"%s %d","test1",55); if ( strcmp(ress_nohdr, "test1 55\n") ) return err.pushl( __LINE__ , "on log.push cmp result");
		log.push(0,"%s %d","test2",66); if (strcmp(ress_nohdr, "test2 66\n")) return err.pushl(__LINE__, "on log.push cmp result");
		log.push(0, "s d"); if (strcmp(ress_nohdr, "s d\n")) return err.pushl(__LINE__, "on log.push cmp result");
		int msten[] = { opstat_l._count_op_alloc , opstat_l._count_op_free };
		if (msten[0] != mstst[0] || msten[1] != mstst[1]) // its use heap - tis is error
			err.pushl(__LINE__, "invalid mem operation count");

		//std::ostream ss;ss << resl;
		//std::cout << resl;

		return 0;
	}
	static int test_log_file(test_state& err) {
		return 0; // TODO:! настройит работу с файлами во время теста!
		log_object_t log;
		stringA resl;  resl.reserve(1024);
		//auto fn = "d:/temp/logtest/test_log.txt";
		//auto fn = "d:\\temp\\test_log.txt";
		auto fn = "%temp%/tests/test_log.txt";
		log.log_open( fn, "pref", false , log_decls_t::mode_TIME_HL | log_decls_t::mode_NoAppend);
		if (!log.isFile()) return err.pushl(__LINE__, "on log.open ");
		auto redir = [&](log_msg_t* m, const char* d, size_t sz) {	resl.assign(d, sz);	};
		log.add_redirector(redir);
		log.push(0, "%s %d", "test1", 55); 
		log.push(0, "%s %d", "test2", 66); 
		log.push(0, "s d"); 
		return 0;
	}
	//static stringA rrr="test of cs";
	static int test_def_log(test_state& err) {
		//std::cout << rrr;
		return 0;
		auto& log = *log_decls_t::getcurrentlog();
		log.push(0, "default log start %s %d", "test1", 55);
		LOG(11, "default log start %s %d", "test1", 55);
		LOG("default log start %s %d", "test1", 55);
		return 0;
	}


	// rc_array
mDeclareTestProc(test_log,esr_TestNeedUpdate ){
	//int test_log() {
		
		
		if (call_test(test_def_log, "test_def_log")) return 1;
		if (call_test(test_log_file, "test_log_file")) return 1;
		if (call_test(test_log_localb, "test_log_localb")) return 1;
		return 0;
	}

}}

