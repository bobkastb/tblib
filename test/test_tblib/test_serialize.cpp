/*
#include "test_darray.h"
static t_operation_statistic opstat_l(_DBG_ARR_START, _DBG_ARR_END, t_operation_statistic::get_DBG_ARR_names());
#define DEBUG_TECH_ARRAY(opcode,mask) opstat_l.onUseOperation(opcode, mask);
*/
#include "gdata/tb_serialize.h"
#include "test_state.h"

namespace tbgeneral { namespace test_ns {

struct s_test_flat_t { 
	char si8; int16_t si16; int32_t si32; int64_t si64;
	uint8_t ui8; uint16_t ui16; uint32_t ui32; uint64_t ui64;
	darray<char> dai8; darray<int64_t> dai64;
	darray<stringA> sa;
};

template<typename Et> int cmp( const darray<Et> & l , const darray<Et>& r) {
	auto lsz = l.size(); auto rsz = r.size();
	if (lsz!=rsz)  return lsz<rsz ? -1 : 1;
	for (size_t i=0;i<lsz;i++) 
		if (l[i]!=r[i]) return l[i]<r[i] ? -1 : 1;
	return 0;
}

struct s_test1_t{
	s_test_flat_t flat;
	s_test1_t* left;
	s_test1_t* right;
	s_test1_t() { left =right=0;}
	s_test1_t(const s_test_flat_t & _flat , s_test1_t* l , s_test1_t* r) { left =l; right = r; flat = _flat; }
};
int cmps(s_test1_t & l , s_test1_t& r){
	if (l.flat.si8!= r.flat.si8) return 1;
	if (l.flat.si16 != r.flat.si16) return 2;
	if (l.flat.si32 != r.flat.si32) return 3;
	if (l.flat.ui8 != r.flat.ui8) return 4;
	if (l.flat.ui16 != r.flat.ui16) return 5;
	if (l.flat.ui32 != r.flat.ui32) return 6;
	if (l.flat.si64 != r.flat.si64) return 7;
	if (l.flat.ui64 != r.flat.ui64) return 8;
	if ( cmp( l.flat.sa , r.flat.sa ) ) return 10;
	if (cmp(l.flat.dai8, r.flat.dai8)) return 11;
	if (cmp(l.flat.dai64, r.flat.dai64)) return 12;
	if ( (l.left!=0) != (r.left!=0) ) return 20;
	if ((l.right != 0) != (r.right != 0)) return 21;
	return 0;
}
t_serializer& operator << (t_serializer& st, const s_test_flat_t& v) {
	st << v.si8 << v.si16 << v.si32 << v.si64;
	st << v.ui8 << v.ui16 << v.ui32 << v.ui64;
	st << v.dai8  << v.dai64 << v.sa; 
	return st;  }
t_serializer& operator >> (t_serializer& st, s_test_flat_t& v) {  
	st >> v.si8 >> v.si16 >> v.si32 >> v.si64;
	st >> v.ui8 >> v.ui16 >> v.ui32 >> v.ui64;
	st >> v.dai8 >> v.dai64 >> v.sa;
	return st; }

static int errinser_crc=0;

t_serializer& operator << (t_serializer& st, const s_test1_t & v) { 
	auto startofs = st.make_start_crc();
	st << v.flat;
	st << int32_t((v.left?1:0)|(v.right?2:0));
	if (v.left) st << *v.left;
	if (v.right) st << *v.right;
	st.fix_end_crc(startofs);

	return st;
}
t_serializer& operator >> (t_serializer& st, s_test1_t& v) { 
	
	if (!st.check_crc()) {
		errinser_crc = 1; return st;
	};
	int32_t lre;
	v.left = 0; v.right = 0;
	st >> v.flat; 
	st >> lre;
	if (lre & 1) { auto p = new s_test1_t(); st>>*p; v.left=p;	}
	if (errinser_crc) return st;
	if (lre & 2) { auto p = new s_test1_t(); st >> *p; v.right = p;}
	if (errinser_crc) return st;

	return st;
}


static int test_start_0(test_state& err) { 
	s_test_flat_t f1= {
		//char si8; int16_t si16; int32_t si32; int64_t si64;
		char(0x88),0x1616, 0x32323232,	0x6464646464646464,
		//uint8_t ui8; uint16_t ui16; uint32_t ui32; uint64_t ui64;
		0x98,0x3636, 0x72727272,0x7474747474747474,
		//darray<char> dai8; darray<int64_t> dai64;
		{ 0,1,2,3,4,5,6,7,8,9,10},
		{0x1001,0x1002,0x1003,0x1004,0x1005,0x1006,0x1007,0x1008}
		//darray<stringA> sa;
		,{"s-1","s-2","s-3","s-4","s-5","s-6","s-7","s-8",}

	};
	s_test1_t tin(	f1, new s_test1_t(f1,0, new s_test1_t(f1,0,0)), 0 );
	//s_test1_t tin = {};

	t_mem_serializer twr(10*1024);
	twr << tin;

	s_test1_t tout;
	t_mem_serializer trd( twr.buff);
	trd >> tout;
	if (errinser_crc) 
		return err.pushl(__LINE__,"crc invalid!");
	if (auto e=cmps( tin , tout )) 
		return err.pushl(__LINE__, "no valid unser data!");
	return 0; 
}


mDeclareTestProc(test_serialize, esr_TestIsReady){
//int test_serialize() {
	if (call_test(test_start_0, "test_start_0")) return 1;
	return 0;
}

}};
