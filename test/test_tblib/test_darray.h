#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <map>
#include "gdata/tb_basedtools.h"
#include "test_state.h"



enum {
	_DBG_SHDRALLOC = 1,
	_DBG_ARR_START = 0x10
	, _DBG_ARR_ASSIGNTHIS = _DBG_ARR_START, _DBG_ARR_ASSIGN_DATA, _DBG_ARR_INSERT, _DBG_ARR_ERASE, _DBG_ARR_MAKEUNIQUE
	, _DBG_ARR_MAKENEW, _DBG_ARR_REPACK, _DBG_ARR_RESIZE, _DBG_ARR_RESERVE
	, _DBG_ARR_END
};

struct t_operation_statistic {
	enum { opMaskSize = 16, opMaskAll = 0xFFFF };
	using mapnames_t = std::map<size_t, std::string>;
	bool isinit = false;
	size_t start_operation;
	int _count_op_alloc = 0;
	int _count_op_free = 0;
	mapnames_t mapNames;
	std::vector<uint32_t> mapop;
	std::vector<uint32_t> countop;
	//t_operation_statistic(){}
	void onUseOperation(size_t opcode, size_t mask);
	~t_operation_statistic();
	t_operation_statistic(size_t beginOPC, size_t endOPC, const mapnames_t& mn);
	void reset();
	int checkFull(std::ostream& outdiag);
	static mapnames_t get_DBG_ARR_names();
};

namespace tbgeneral {
	namespace test_ns {



enum eOperationCode {
	eop_Insert = 1, eop_Append = 2, eop_Erase = 3, eop_Resize = 4, eop_Reserve = 5, eop_Construct, eop_AssignIntersect
	, eop_Assign, eop_MakeUnique, eop_InsertIntersect , eop_Trim
};
enum ePreOperationFlags { efNone = 0, efRESERVE = 1, efREFS = 2, efCnstSeg = 4, 
	efwChangeStart=0x10 , efwRealloc = 0x20 , efwStorageZero= 0x40
	,efTestMemRes=0x10000 , efNoCheckResultData = 0x20000
};
struct test_operations_loc_t {
	size_t start, size;
};

template<typename TChar> struct test_datachars_tt {
	eOperationCode operationcode;
	const TChar* source;
	std::vector<test_operations_loc_t> preSlice;
	const TChar* opdata;
	std::vector<test_operations_loc_t> operloc;
	const TChar* opresult;
	size_t oflags; //ePreOperationFlags
};
template<typename TChar> struct test_datachars_one_t {
	eOperationCode operationcode;
	const TChar* source;
	test_operations_loc_t preSlice;
	const TChar* opdata;
	test_operations_loc_t operloc;
	const TChar* opresult;
	size_t oflags; //ePreOperationFlags
};

using test_datachars_t = test_datachars_tt<char>;
/*
struct test_datachars_t {
	eOperationCode operationcode;
	const char* source;
	std::vector<test_operations_loc_t> preSlice;
	const char* opdata;
	test_operations_loc_t operloc;
	const char* opresult;
	//ePreOperationFlags
};
*/
std::string get_operation_presentation(eOperationCode operationcode, test_operations_loc_t loc, const char* opdata);

}};
