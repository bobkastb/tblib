/*
#include "test_darray.h"
static t_operation_statistic opstat_l(_DBG_ARR_START, _DBG_ARR_END, t_operation_statistic::get_DBG_ARR_names());
#define DEBUG_TECH_ARRAY(opcode,mask) opstat_l.onUseOperation(opcode, mask);
*/

#include "test_state.h"
#include "gdata/pack/ArithmeticCoding.h"
#include "gdata/pack/ArithmeticCoding_learn.h"
#include "gdata/tb_map.h"


#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-function"
#endif










namespace tbgeneral { namespace test_ns {

static int test_ac_one(test_state& err , int index ,  c_arithmetic_coding_base  &ac, const stringW & idata , size_t comp_size ){
	ac.encode_start(); // не обязательно
	auto sc = ac.code2string<stringW>( ac.string2code(idata) );
	if (sc != idata) return err.pushf("line(%d,%d) string2code error" , __LINE__,index);
	auto edata = ac.encode_string(idata);
	auto dedata= ac.decode_string<stringW>( edata );
	if (dedata!=idata) return err.pushf("line(%d,%d) encode decode error" , __LINE__,index);
	if (edata.size()!=comp_size) 
		return err.pushf("line(%d,%d) encode size error. [%d]!=%d  " , __LINE__,index , edata.size() , comp_size);
	return 0;
}





static const char *  jsontest_alphabets=u8R"const(
{
"learn":{"path": "./data/text/en_ru/lt1.txt" },
"outfile":"./data/freqs/testac_result.json",
"ac_params":{"start_alpha":1,"alpha_has_terminal_char":1,"minimum_frequency":1, "rate_endchar":0.0005 },
"alphabets":[
	{	"id":1,
		"lang":"ru",
		"symbols":36,
		"alphabet":" ,.абвгдеёжзийклмнопрстуфхцчшщъыьэюя", 
		"ngramm":2}
	,{ 	"id":2,
		"lang":"en", 
		"symbols":29, 
		"alphabet":" ,.abcdefghijklmnopqrstuvwxyz",
		"ngramm":2}
	,{ 	"id":3,
		"lang":"ru.UCASE",
		"symbols":33,  
		"alphabet":"АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ",
		"Single":true}
	,{ 	"id":4,
		"lang":"en.UCASE",
		"symbols":26, "1/26":0.03846,
		"alphabet":"ABCDEFGHIJKLMNOPQRSTUVWXYZ",
		"Single":true}
	,{ 	"id":5,
		"lang":"Цифры",
		"symbols":14, "1/14" : 0.07142,
		"alphabet":" .:-0123456789"}
	,{ 	"id":6,	
		"lang":"Другие символы",
		"symbols":39, "1/39":0.02564,
		"alphabet":" (){}[]&*+-/=<>^|~!,.:;?«»\"#$%@§©·№_±\\",
		"Single":true}
	]
}
)const";	


static const char *  text_learn1=u8R"const(
	zzzzz zzzzz zzzzz zzzzz zzzzz zzzzz
	ccd ccd ccd ccd ccd ccd ccd ccd ccd ccd ccd ccd ccd ccd ccd 
	111 222 333 444 555 666 777 888 999 000 
	,.абвгдеёжзийклмнопрстуфхцчшщъыьэюя
	,.abcdefghijklmnopqrstuvwxyz
)const";

//-------- test LEARN ngramm -----------

struct test_ac_ngram_t { stringA setting , result , data ; };
 
//#include "text_learn1_result.json.inc"
#include "text_learn2_result.json.inc"

#include "text_learn3_result.json.inc"

//auto tt1 = "ddd" "yyyy";
using t_AC= c_arithmetic_coding_ngramm;

test_ac_ngram_t tests_ac_ngram[] ={
		{ ns_aclearn4::ac_setting , ns_aclearn4::ac_learn_result , ns_aclearn4::ac_learn_data }
		,{ ns_aclearn2::ac_setting , ns_aclearn2::ac_learn_result , text_learn1 }
		,{ jsontest_alphabets , "" , text_learn1 }
		,{ ns_aclearn3::ac_setting , ns_aclearn3::ac_learn_result , ns_aclearn3::ac_learn_data }
	
};

static int test_ac_ngramm_learn( test_state& err ) {
	int index = 0;
	for ( auto tst : tests_ac_ngram ) {
		t_ac_text_learn lrn;
		if ( lrn.InitFrom( tst.setting ) ) 
			return err.pushl(__LINE__," i(%d) error load json" , index);
		if ( lrn.make_from_data( tst.data ) ) 
			return err.pushl(__LINE__," i(%d) error learn", index);
		stringA buff;
		//if ( lrn.saveresult("c:/temp/lt2.json" , t_AC::eBJO_FloatTransition | t_AC::eBJO_DefaultMask) ) 
		if (lrn.saveresult_buff(buff , t_AC::eBJO_FloatTransition | t_AC::eBJO_DefaultMask ))
			return err.pushl(__LINE__," i(%d) error save to json", index);
		//auto sTr = stringA(text_learn2_result);
		if ( !tst.result.empty() && buff!= tst.result )
			return err.pushl(__LINE__," i(%d) error invalid ac learn result ", index);
		index++;
	}

	return 0;
} 

//-------- test ngramm -----------

t_AC* new_ac_m( const stringA & jsondata ) {
	auto pac = new t_AC();
	pac->loadjson( 0 , jsondata );
	return pac;
}

static int test_ac_ngramm_loadstore( test_state& err ) {
	return 0;
}
struct test_ac_ngramm_t {
	stringA ac_setting , in_text ;
	size_t compress_size;
};
static int test_ac_ngramm( test_state& err ) {
	test_ac_ngramm_t tests[] = {
		{ns_aclearn4::ac_learn_result ,  "aaaaaaaaaaaaaaaaaaaaaaaaa" , 5 } //25
		,{ns_aclearn3::ac_learn_result , "aaaaaaaaaaaaaaaaaaaaaaaaa" , 5 } //25
		,{ns_aclearn3::ac_learn_result , "bbbbbbbbbbbbbbbbbbbbbbbbb" , 44 } //25 переход b->b маловероятен, поэтому сжатия не случилось
		,{ns_aclearn3::ac_learn_result , "abcde abcde abcde abcde a" , 4 } //25
		,{ns_aclearn3::ac_learn_result , "aaaaaaaaaa0123456789aaaaa" , 10 } //25
	};
	std::map<stringA,t_AC*> acmap; 
	int index = 1000;
	for (auto tst: tests) {
		if (!exists(acmap, tst.ac_setting)) { }
		auto acp = acmap[tst.ac_setting];
		if (!acp) { acp = new_ac_m(tst.ac_setting);
			if (acp->error.error_id) return err.pushl(__LINE__,"i(%d) invalid ac setting",index);
			acmap[tst.ac_setting] = acp; 
		}
		if (test_ac_one( err , index ,  *acp , tst.in_text , tst.compress_size)) return 1;
		index++;
	}
	for (auto em : acmap)
		delete em.second;

	return 0;
}


//------------ test multi alphabet
using r_symbol_freq = r_symbol_freq_m<wchar,double>;
using extrai = r_alphabet_extrainfo<wchar,double>;

static darray<r_symbol_freq> initfreqs_simpl( const stringW & ws ) {
	darray<r_symbol_freq> res; res.resize(ws.size());
	auto ressz = res.size();
	uint i=0; FOREACH( pw, ws) { res[i]=r_symbol_freq(*pw, double(1)/ressz ); i++; };
	return res;
};

static extrai prep_ei( const extrai & _ei){
	return _ei;
	extrai ei= _ei;
	double mst = 1.0/ei.rates.size();
	//for (uint i=0;i<ARRAYLEN(ei.transit);i++ ){ 
	for ( auto &t : ei.transit) {
		//auto p=&ei.transit[i];
		if (t.to_id)  
			t.rate*=mst; 
	}
	return ei;
}

//static void test_array_param(int x , const int y[100] ) {}

static void ac_init( c_arithmetic_coding_m  &ac ){
	ac.sv.set_CodeLen(16);
	auto enl=L" ,.abcdefghijklmnopqrstuvwxyz";
	auto rul=L" ,.абвгдеёжзийклмнопрстуфхцчшщъыьэюя";
	auto genl=L"()-_.?,:;\"";
	auto nums="0123456789.";
	typedef const extrai::t_transit t_tr[20];
	t_tr tra[] = { 
		{ {2,0.5},{3,0.2},{4,0.1} } 
		,{ {1,0.3},{3,0.2},{4,0.1} }
		,{ {1,0.4},{2,0.5},{4,0.1} }
		,{ {1,0.4},{2,0.5},{3,0.1} }
	};
	//t_tr{ {4,55} };
	//t_tr x{ {2,0.5},{3,0.2},{4,0.1} };
	//extrai( 1, initfreqs_simpl(rul) , 100.0/6, false, t_tr { {2,0.5},{3,0.2},{4,0.1} }  );

	ac.add_alphabet( prep_ei(extrai( 1, initfreqs_simpl(rul) , 100.0/6, false, tra[0] )));
	ac.add_alphabet( prep_ei(extrai( 2, initfreqs_simpl(enl) , 150.0/6, false, tra[1] )));
	ac.add_alphabet( prep_ei(extrai( 3, initfreqs_simpl(nums), 90.0/2,	false, tra[2] )));
	ac.add_alphabet( prep_ei(extrai( 4, initfreqs_simpl(genl), 2.0/1,	true,  tra[3] )));
}


static int test_c_arithmetic_coding_m(test_state& err){
	struct test_t{ stringW intxt ; size_t comp_size; };
	test_t tests[]={
		{L"aaaaa aaaaa,aaaaa aaaaa,aaaaa aaaaa,aaaaa aaaaa,aaaaa aaaaa," , 39 }
		,{L"aaaaa" , 6 }
		,{L"a" , 3 }
		,{L"test encoding. 918550 бвбвбвбвбвбв" , 25 }
	};
	c_arithmetic_coding_m ac;
	ac_init( ac ) ;
	int index=0;
	for (auto tst : tests){
		if (test_ac_one(err,index, ac , tst.intxt , tst.comp_size )) return 1;
		index++;
	}
	return 0;
}










struct ac_test_t {
	stringW alphabet;
	stringW intext;
	size_t compressed_size;
	darray<c_arithmetic_coding_c::rchar_freq> freqs;
};

static int test_ac_f(test_state& err , int index, const ac_test_t & tst) {
	c_arithmetic_coding_c ac;
	if (tst.freqs.empty()) {
		ac.int_freq_equiprobable( tst.alphabet );
	} else {
		ac.set_freqs( tst.freqs.data() , tst.freqs.size() );
	}
	//stringW intext = L"test encoding 918";

	ac.prepare_alphabet_set();
	//auto codelist = ac.string2code( tst.intext );
	ac.encode_start();
	//printACparams(ac);
	auto dres  = ac.encode_string(tst.intext);
	//c_printf(" encode bitscount= %d\n" , ac.wv.bitstream.countbits_in );
	auto compr_size = dres.size();
	auto tres = ac.decode_string<stringW>( dres );
	//c_printf(" decode bitscount= %d\n" , ac.wv.bitstream.countbits_in );
	if (tst.intext!=tres) 
		return err.pushf("line(%d,%d) invalid decode result", __LINE__, index );
	if (tst.compressed_size!=0 && compr_size!= tst.compressed_size ) 
		return err.pushf("line(%d,%d) invalid compressed size", __LINE__, index );
	return 0;
}

static int test_c_arithmetic_coding_c(test_state& err){
	stringW alphaEn = L" 0123456789abcdefghijklmnopqrstuvwxyz.,";
	ac_test_t tests[]={
		{ "" , L"ttttattttattttatttta" , 3 , { {'t',4},{'a',1} }}
		,{ L"t" , L"tttttttttttttttttttt" , 1 } // 20 chars
		,{ alphaEn , L"test encoding 918" , 13 }
	};
	int index=0;
	for( auto tst : tests ){
		if (test_ac_f( err, index, tst )) return 1;
		index++;
	}

	return 0;
}




mDeclareTestProc(test_ArithmeticCoding,esr_TestNeedUpdate){
//int test_ArithmeticCoding() {
	if (call_test(test_c_arithmetic_coding_m, "test_c_arithmetic_coding_m")) return 1;
	if (call_test(test_ac_ngramm, "test_ac_ngramm")) return 1;
	if (call_test(test_c_arithmetic_coding_c, "test_c_arithmetic_coding_c")) return 1;
	if (call_test(test_ac_ngramm_learn, "test_ac_ngramm_learn")) return 1;
	if (call_test(test_ac_ngramm_loadstore, "test_ac_ngramm_loadstore")) return 1;
	
	
	
	return 0;
}

}};
