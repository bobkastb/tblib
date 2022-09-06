/*
#include "test_darray.h"
static t_operation_statistic opstat_l(_DBG_ARR_START, _DBG_ARR_END, t_operation_statistic::get_DBG_ARR_names());
#define DEBUG_TECH_ARRAY(opcode,mask) opstat_l.onUseOperation(opcode, mask);
*/

#include "test_state.h"

namespace tbgeneral { namespace test_ns {

static int test_start_0(test_state& err) { return 0; }


mDeclareTestProc(test_sys_service,esr_TestIsAStub){
//int test_sys_service() {
	if (call_test(test_start_0, "test_start_0")) return 1;
	return 0;
}

}};
