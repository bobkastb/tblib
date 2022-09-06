/*
#include "test_darray.h"
static t_operation_statistic opstat_l(_DBG_ARR_START, _DBG_ARR_END, t_operation_statistic::get_DBG_ARR_names());
#define DEBUG_TECH_ARRAY(opcode,mask) opstat_l.onUseOperation(opcode, mask);
*/

#include "test_state.h"
#include "gdata/cipher/blockCipher.h"
#include "gdata/tb_numbers.h"

namespace tbgeneral { namespace test_ns {



darray<byte> str2Arr(const stringA & s) { return darray<byte>( (const byte*) s.cbegin() , s.size());  }


static stringA alignstrsize( const stringA & s , size_t sz ){
	auto datain = s;
	while (datain.size() < sz ) datain << s;
	if (datain.size() > sz ) datain.resize(sz);
	return datain;
}

static int test_start_0(test_state& err) { 
	auto unikey = stringA("0123456789abcdef");
	auto datas = stringA("0-1-2-3-4-5-6-7-");
	auto iv = str2Arr("0123456789abcdef");

	struct test_chif_t { c_AES_base::eKeySizeB keysz; c_AES_base::eMethod meth; stringA resultH; };
	test_chif_t tests[]={ 
		//onlain check AES:  https://www.devglan.com/online-tools/aes-encryption-decryption
		{c_AES_base::bAES_192,c_AES_base::mCBC , "32F32D625F9A18A19DD012C055466893"}
		,{c_AES_base::bAES_192,c_AES_base::mECB ,"C6895E365EB54A20C4924E46BD6C6756"}
		,{c_AES_base::bAES_192,c_AES_base::mCTR , ""}
		,{c_AES_base::bAES_256,c_AES_base::mCBC , "4A3FAF7CB7A08F0E3D786D9D4C5E9D84" }// "4A3FAF7CB7A08F0E3D786D9D4C5E9D84DB1D8C5ECD527A6E33E989188C560EE2"}
		,{c_AES_base::bAES_256,c_AES_base::mECB ,"4ED7D6110184F352E4CF8809F9AA34C1"} //"4ED7D6110184F352E4CF8809F9AA34C14ED7D6110184F352E4CF8809F9AA34C1"} 
		,{c_AES_base::bAES_256,c_AES_base::mCTR , ""}
		,{c_AES_base::bAES_128,c_AES_base::mECB ,"7CDFF3AD2DAD8EA60690647E0727BEE5"}
		,{c_AES_base::bAES_128,c_AES_base::mCBC , "758367777D494FEC7DE655BC15F71425"}
		,{c_AES_base::bAES_128,c_AES_base::mCTR , ""}
	};
	int linen=0;
	for (auto tst : tests) {
		auto datain = datas.copy(); //alignstrsize( datas , tst.keysz ) ;
		auto key = cChipherKey( alignstrsize(unikey, tst.keysz) , tst.keysz);

		auto datac = str2Arr(datain);
		auto chif = c_AES_base::Make( tst.meth , key , iv);
		chif->full_encode(datac.begin(), datac.size());
		auto datar = arr2hex(datac);
		if ( (!tst.resultH.empty()) && (datar != tst.resultH) )
			return err.pushl(__LINE__, " (%d) chifer error result!" , linen);
		chif->full_decode(datac.data(), datac.size());
		if (0!=memcmp(datain.cbegin() , datac.cbegin() , datac.size() ))
			return err.pushl(__LINE__, " (%d) chifer error back transform!", linen);
		linen++;
	}
	return 0;
}


mDeclareTestProc(test_chifer, esr_TestIsReady){
//int test_sys_process() {
	if (call_test(test_start_0, "test_start_0")) return 1;
	return 0;
}

}};
