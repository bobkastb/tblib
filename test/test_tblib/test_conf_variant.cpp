#include "conf/conf_variant.h"
/*
#include "test_darray.h"
static t_operation_statistic opstat_l(_DBG_ARR_START, _DBG_ARR_END, t_operation_statistic::get_DBG_ARR_names());
#define DEBUG_TECH_ARRAY(opcode,mask) opstat_l.onUseOperation(opcode, mask);
*/

#include "test_state.h"
//namespace tbgeneral {	void test_cv_111();};
namespace tbgeneral {	namespace test_ns {


static conf_struct_ref _list(const darray<conf_variant>& l) {
		auto jn = conf_struct_t::newnode(conf_variant::ejtArray);
		jn->a_data = l;
		return jn;
}
static conf_struct_ref _obj(const darray<conf_struct_t::r_property>& l) {
		auto jn = conf_struct_t::newnode(conf_variant::ejtObject);
		jn->named_props = l;
		return jn;
}

static int test_start_0(test_state& err) { 
	//auto xx= conf_struct_t::r_property{ "rr",1 };
	//auto root = _obj({ xx });
	auto root = _obj({ {"f01", 66}, {"f02", 77}, 
		{ "r01" , _obj({{"f11", 88}, {"f12", 99}}) }
		});
	if (root->get<int>("f01") != 66)
		return err.pushl(__LINE__, " e1 ");
	if (root->get<int>("f01-") != 0)
		return err.pushl(__LINE__, " e1 ");
	if (root->get_path<int>("r01/f11") !=88)
		return err.pushl(__LINE__, " e1 ");
	//if (root->get_path<int>("r01-/f11") != 0) return err.pushl(__LINE__, " e1 ");
	return 0;
}



mDeclareTestProc(test_conf_variant, esr_TestNeedUpdate ) {
	//int test_net() {
	if (call_test(test_start_0, "test_start_0")) return 1;
	//test_cv_111();
	return 0;
}

}};
