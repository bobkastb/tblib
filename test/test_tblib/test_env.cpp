/*
#include "test_darray.h"
static t_operation_statistic opstat_l(_DBG_ARR_START, _DBG_ARR_END, t_operation_statistic::get_DBG_ARR_names());
#define DEBUG_TECH_ARRAY(opcode,mask) opstat_l.onUseOperation(opcode, mask);
*/

#include "test_state.h"
#include "gdata/tb_env.h"
#include "gdata/t_format.h"


//get_all_environmentA()
namespace tbgeneral { 
void debug_set_new_environ( const char ** eA ,  const wchar_t ** eW   );

}

namespace tbgeneral { namespace test_ns {

stringA env2initext( const LocalEnv & env , char sep=';'){
	char buffer[4*1024];
	stringA res; res.assign_storage(buffer);
	for (auto pr : env.index) {
		res << pr.first << '=' << pr.second << sep;
	}
	res.free_external_storage(buffer);
	return res;
}

static const char * dbgdataA[]={
	"var0=value0","var1=value1","var2=value2","var3=value3","var4=value4","var5=value5",0
};
static bool isSLiceOf( const stringA & v , const stringA & container){
	return v.cbegin()>=container.cbegin() && v.cend()<=container.cend();
}

static int test_start_0(test_state& err) { 
	debug_set_new_environ( dbgdataA , 0); 
	stringA allenv;
	get_all_environment(allenv);
	auto prgenv = get_programm_env();
	for (auto p : prgenv->index ) {
		auto k = p.first; auto v=p.second;
		if (!isSLiceOf(k , allenv ) || !isSLiceOf(v , allenv ) ) 
			return err.pushf("line(%d) invalid mem location key-value for %s", __LINE__, k.cbegin() );
	};
	return 0;
}
static int test_expand(test_state& err) { 
	stringA res , name ,wr;
	auto prgenv = get_programm_env();
	//int replcnt;
	res = getEnvVar( "var2"  );
	for (int i=0;i<std::size(dbgdataA);i++) if (dbgdataA[i]) {
		name = format( "var%d" ,i);
		wr =format( "value%d" ,i);
		res = getEnvVar( name  );
		if (wr!=res) 
			return err.pushl(__LINE__,"get environment variable error");
	}
	expand_environment_string( "aaa %% bbbb %var0% ccc %var1% ddd %error% eee" , res );
	if (res!="aaa % bbbb value0 ccc value1 ddd %error% eee") return err.pushl(__LINE__,"expand error");
	expand_environment_string( "%% bbbb %var0% ccc %var1% ddd %error%" , res );
	if (res!="% bbbb value0 ccc value1 ddd %error%") return err.pushl(__LINE__,"expand error");
	expand_environment_string( "%var0% %% ccc %error% ddd %var1%" , res );
	if (res!="value0 % ccc %error% ddd value1") return err.pushl(__LINE__,"expand error");
	return 0;
}

struct clos_test_t{
	LocalEnv::eclos_Result res ; const char * in ; const char * out;
};

int test_closure( test_state& err , int lineno, int index , clos_test_t & t ){
	LocalEnv lenv;
	lenv.loadfrom_compact_text(t.in,';');
	auto clr = lenv.closure_ext();
	if (clr!=t.res)
		return err.pushf("line:(%d%d)invalid closure result (%d!=%d)", index, lineno, clr,t.res);
	auto ftxt = env2initext( lenv , ';');
	if (ftxt!=t.out) 
		return err.pushf("line:(%d%d)invalid closure result data", index, lineno);
	return 0;
}

static int test_closure(test_state& err) { 
	const char * dbgdata_1="param0=%var0%/utter;param1=butter/%param0%/butter;param3=tester3;param4=%param0%/%param1%/%param3%;";
	LocalEnv env1;
	env1.loadfrom_compact_text(dbgdata_1,';');
	if (env1.get("var0")!="value0") 
		return err.pushl(__LINE__,"invalid redirect");
	auto clr= env1.closure_ext();
	if (clr) return err.pushl(__LINE__,"invalid closure");
	if (env1.index["param0"]!="value0/utter") 
		return err.pushl(__LINE__,"invalid closure");
	auto ftxt = env2initext( env1 , ';');
	if (ftxt!="param0=value0/utter;param1=butter/value0/utter/butter;param3=tester3;param4=value0/utter/butter/value0/utter/butter/tester3;")
		return err.pushl(__LINE__,"invalid closure data");
	clos_test_t tests[]={
		{LocalEnv::eclos_NoExistVar,"param0=(%param1%);param1={%noexist%};"
		,"param0=({%noexist%});param1={%noexist%};"}
		,{LocalEnv::eclos_CircularReference , "param0=(%param1%);param1={%param0%};",
		"param0=({%param0%});param1={({%param0%})};"}
		,{LocalEnv::eclos_CircularReference , "param0=y/%param2%/r;param1=x/%param0%/y;param2=a/%param1%/b;",
		"param0=y/a/x/y/a/%param1%/b/r/y/b/r;param1=x/y/a/%param1%/b/r/y;param2=a/x/y/a/%param1%/b/r/y/b;"}
	};
	for (size_t i=0;i<std::size(tests);i++){
		if (test_closure(err, __LINE__, (int) i , tests[i] )) return 1;
	}
	return 0;
}

//mDeclareTestProc(test_env,esr_TestIsAStub){
mDeclareTestProc(test_env,esr_TestIsReady ){
//int test_env() {
	if (call_test(test_start_0, "test_start_0")) return 1;
	if (call_test(test_expand, "test_expand")) return 1;
	if (call_test(test_closure, "test_closure")) return 1;
	return 0;
}

}};
