#include "tsys/tb_sysdecls.h"
#include "gdata/tb_numbers.h"
/*
#include "test_darray.h"
static t_operation_statistic opstat_l(_DBG_ARR_START, _DBG_ARR_END, t_operation_statistic::get_DBG_ARR_names());
#define DEBUG_TECH_ARRAY(opcode,mask) opstat_l.onUseOperation(opcode, mask);
*/

#include "test_state.h"

namespace tbgeneral { namespace test_ns {


//===============================================
struct str2number_t {
	stringA txt;
	int64 value;
	int vtype;
	int res;
};
static int str2number_f( str2number_t t ){
	int res=0;
	int64 dres; 
	uint8_t b1; int8_t b_1; uint16_t b2; int16_t b_2; uint32_t b4; int32_t b_4; uint64_t b8; int64_t b_8;
	switch (t.vtype) {
		case 1: res = str2number( t.txt , &b1); dres=b1;  break;
		case -1: res = str2number(t.txt, &b_1); dres=b_1;  break;
		case 2: res = str2number(t.txt, &b2); dres = b2;  break;
		case -2: res = str2number(t.txt, &b_2); dres = b_2;  break;
		case 4: res = str2number(t.txt, &b4); dres = b4;  break;
		case -4: res = str2number(t.txt, &b_4); dres = b_4;  break;
		case 8: res = str2number(t.txt, &b8); dres = b8;  break;
		case -8: res = str2number(t.txt, &b_8); dres = b_8;  break;
	}
	if ( t.res != res ) return 1;
	if ( res!=0 ) return 0;
	if ( t.value != dres ) return 2;
	return 0;
}

static int str2number_bigtest(test_state& err) { 
	str2number_t tests[]={ 
		{"0x1234",0,1, str2number_c::Overflow}, 
		{"-32.768E3",	-32768,	-2, str2number_c::Ok},
		{"32.767E3",	32767,	-2, str2number_c::Overflow},
		{"32.7669E3",	32766,	-2, str2number_c::Ok},
		{"0x1234",		0x1234,	-2, str2number_c::Ok},
		{"2147483647",	2147483647,	-4, str2number_c::Ok},
		{"4294967295",	4294967295,	4, str2number_c::Ok},
		{"4294.967295E+6",	4294967295,	4, str2number_c::Overflow},
		{"4294.9672949E+6",	4294967294,	4, str2number_c::Ok},
		{"0x1234",		0x1234,	4, str2number_c::Ok},
	};
	int n=0;
	for ( auto t : tests ){
		auto r = str2number_f( t );
		switch (r) {
		case 1: return err.pushl(__LINE__,"[%d],Error at str2number" , n); 
		case 2: return err.pushl(__LINE__, "[%d],Error at str2number, get value no match", n);
		}
		n++;
	}
	return 0;
};

warning_MSVC(disable,4101)

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wsign-compare"

#endif

void test_ConvertSimpleT(){
	//typedef unsigned int8 u_int8;
	//wprintf();
	//str2number_bigtest();

	enum { VVm=int8(-1)<0? 99 :88 , VVb= -128 };
	{ bool r= int8(-1) < 0 ;
	  int xx=	VVb;
	  r=false;	
	};
	{
	int64 vi64; uint64 vui64; int r;
	int32 vi32; uint32 vui32;
	 r= convertnum_CHKOVER( vi32 , uint32(-1) );
	 r= convertnum_CHKOVER( vi32 , uint32(1) );
	 r= convertnum_CHKOVER( vui32 , int32(-1) );
	 r= convertnum_CHKOVER( vui32 , int32(0) );
	 r= convertnum_CHKOVER( vi32 , int32(-1) );
	 r=0;	
	}
	float f=0;
}


static int test_start_0(test_state& err) { return 0; }

mDeclareTestProc(test_convert,esr_TestNeedUpdate){
//int test_convert() {
	if (call_test(test_start_0, "test_start_0")) return 1;
	if (call_test(str2number_bigtest, "str2number_bigtest")) return 1;
	return 0;
}

}};
