#include "test_state.h"
#include "gdata/hash_functions.h"
#include "gdata/t_format.h"


#include <time.h>

namespace tbgeneral {
	namespace test_ns {

struct hash_test_t {
	cHash_Base::HashClassID hid;
	const char* in;
	const char* out;
};

static const hash_test_t testarr[]={
	 {cHash_Base::SHA256 , "1234567"		, "8BB0CF6EB9B17D0F7D22B456F121257DC1254E1F01665370476383EA776DF414" }
	,{cHash_Base::SHA256 , "1234567890"		, "C775E7B757EDE630CD0AA1113BD102661AB38829CA52A6422AB782862F268646" }
	,{cHash_Base::MD5 , "1234567"		, "fcea920f7412b5da7be0cf42b8c93759" }
	,{cHash_Base::MD5 , "1234567890"		, "e807f1fcf82d132f9bb018ca6738a19f" }
};



		//static void print_bytes( const darray<byte> )
const char*  gethashname(cHash_Base::HashClassID hid){
	switch(hid) {
	case cHash_Base::MD5 : return "MD5";
	case cHash_Base::SHA256: return "SHA256";
	}
	return "Invalid Hash id";
}

static int test_hash_a(test_state& err){
	for (size_t i = 0; i < std::size(testarr); i++) {
		auto tst = testarr[i];
		auto h = tbgeneral::cHash_Base::MakeHash( tst.hid );
		auto r = h->CalcHashS( tst.in );
		if (stricmp(r, tst.out )) 
			return err.pushf("line(%d,%d) Invalid %s (%s) %s!=%s \n " , i, __LINE__ , gethashname(tst.hid), tst.in , r.c_str() , tst.out);
	};
	return 0;
}

mDeclareTestProc(test_hash,esr_TestIsReady){
//int test_hash(){
	if (call_test(test_hash_a, "test_hash_a")) return 1;
	return 0;
}


	}
}; // namespace tbgeneral { namespace test{
