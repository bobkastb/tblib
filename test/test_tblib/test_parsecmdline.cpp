/*
#include "test_darray.h"
static t_operation_statistic opstat_l(_DBG_ARR_START, _DBG_ARR_END, t_operation_statistic::get_DBG_ARR_names());
#define DEBUG_TECH_ARRAY(opcode,mask) opstat_l.onUseOperation(opcode, mask);
*/

#include "test_state.h"
#include "conf/tb_parsecmdl.h"
#include "gdata/t_string.h"

namespace tbgeneral {namespace test_ns {

		static const char* txt_cmd_options = u8R"const(
{
"options":[
	{"keys":["c","cfg"], "type":"filename",	"help":"json файл конфигурации" },
	{"keys":["u","update"], "type":true ,	"help":"только перезапись готовой конфигурации в файлы" },
	{"keys":["m","make"], "type":true ,		"help":"создание статистических частотных таблиц символов из текстов" },
	{"keys":["f","force"], "type":true ,	"help":"обязательное создание (заново) статистических частотных таблиц символов из текстов" },
	{"keys":["id","id_config"], "type":"ne","help":"идентификатор конфигурации" },
	{"keys":["p","pass"], "type":"ne",		"help":"пароль на доступ к конфигурации" } 
] 
}
)const";



static int test_start_0(test_state& err) { 
	const char* args[] = { "0","-c=testfile.json", "-u" , "-m" , "-pass=test" };
	r_cmdline_options cmdl;
	if (cmdl.set_as_json(txt_cmd_options)) { 
		return err.pushl( __LINE__,"%s",cmdl.error.text.c_str());}
	if (cmdl.parse_all( (int) std::size(args), args)) {
		return err.pushl( __LINE__,"%s",cmdl.lastparse_result.error.text.c_str());}
	auto lpr = &cmdl.lastparse_result;
	//lpr.
	stringA opta[] = { "c" , "u" , "m" , "p"};
	stringA optv[] = { "testfile.json" , "" , "" , "test"};
	if (std::size(opta)!=lpr->list.size()) return err.pushl( __LINE__," invalid size");
	for (size_t i=0;i<std::size(opta);i++) {
		auto lv = lpr->list[i];
		if (lv.opt->ids[0] != opta[i] ) return err.pushl( __LINE__," i(%d) invalid key" , i);
		if ( lv.value != optv[i] ) return err.pushl( __LINE__," i(%d) invalid value" , i);
	}
	return 0; 
}


mDeclareTestProc(test_parsecmdline,esr_TestIsReady){
//int test_parsecmdline() {
	if (call_test(test_start_0, "test_start_0")) return 1;
	return 0;
}

}};// namespace tbgeneral {namespace test_ns {
