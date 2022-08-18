/*
#include "test_darray.h"
static t_operation_statistic opstat_l(_DBG_ARR_START, _DBG_ARR_END, t_operation_statistic::get_DBG_ARR_names());
#define DEBUG_TECH_ARRAY(opcode,mask) opstat_l.onUseOperation(opcode, mask);
*/

#include "test_state.h"
#include "gdata/t_string.h"
#include "gdata/tbt_locale.h"
#include "gdata/t_format.h"


#include <locale>


namespace tbgeneral { namespace test_ns {

int test_locale1() {
	int errs=0;

	const char * tt="\xd1\x82\xd0\xb5\xd1\x81\xd1\x82";
	stringA cs(tt);
	stringW w(L"тест");

	{
	auto r = utf8_to_wide( tt , strlen(tt) );
	auto res = w==r;  errs += (res!=0?1:0);
	c_printf("convert %s res:%S need:%S", (w==r ? "OK" : "ERR" ) , r , w );
	}

	auto& fw = std::use_facet<std::ctype<wchar_t>>(std::locale());
	fw.toupper( w.begin() , w.end());
	auto lu8 = std::locale("en_US.utf8");
	auto fc = &std::use_facet<std::ctype<char>>(lu8);
	fc->toupper( cs.begin() , cs.end());

	// std::setlocale(LC_ALL, "en_US.iso88591");

	return errs;
}


static int cmp_ms(const stringW buff , const stringW im ){
	auto sz = im.size();
	auto s= buff.begin(), e=buff.end();
	auto ims = im.begin();
	size_t cntr=0;
	for (;s<e;s+=sz, cntr+=sz){
		sz = std::min<size_t>( e-s , sz);
		if (strcmp( s, ims, sz , sz)) 
			return (int) cntr;
	}
	return 0;
}
static int test_utf8_wchart(test_state& err) { 
	stringA imA = u8"-абвгеёжзийклмнопрстуфхцчшщъыьэюя АБВГЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ-";
	stringW imW = L"-абвгеёжзийклмнопрстуфхцчшщъыьэюя АБВГЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ-";
	stringW res,res1; stringA resA;
	// ؈؉؊
	while (res.size()<9*1024) 
		utf8_to_wide_add(res,imA.begin(),imA.size());
	if (cmp_ms( res, imW )) return err.pushl(__LINE__, "error in big add transform ut8-utf16");
	res.clear(); resA.clear();
	while (resA.size() < 9 * 1024)
		resA.append(imA) ;
	res1 = resA;
	res1.resize(res.size());
	if (res1!=res) return err.pushl(__LINE__, "error in big transform ut8-utf16");
	return 0; 
}
static int test_wchart_utf8(test_state& err) {
	stringA imA = u8"-абвгеёжзийклмнопрстуфхцчшщъыьэюя АБВГЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ-";
	stringW imW = L"-абвгеёжзийклмнопрстуфхцчшщъыьэюя АБВГЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ-";
	stringA res, res1; stringW resC;
	// ؈؉؊
	while (res.size() < 9 * 1024)
		wide_to_utf8_add(res, imW.begin(), imW.size());
	if (cmp_ms(res, imW)) return err.pushl(__LINE__, "error in big add transform ut8-utf16");
	res.clear(); resC.clear();
	while (resC.size() < 9 * 1024)
		resC.append(imW);
	res1 = resC;
	res1.resize(res.size());
	if (res1 != res) return err.pushl(__LINE__, "error in big transform ut8-utf16");
	return 0;
}

template <typename TChar> int test_tolower_t(test_state& err) {
	rc_string<TChar> ns = L"-абвгеёжзийклмнопрстуфхцчшщъыьэюя АБВГЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ-abcdef-ABCDEF";
	rc_string<TChar> res = L"-абвгеёжзийклмнопрстуфхцчшщъыьэюя абвгеёжзийклмнопрстуфхцчшщъыьэюя-abcdef-abcdef";
	auto lc=strlower(ns);
	auto nmtype= sizeof(TChar)==1?"char":"wchar_t";
	if (lc!=res) return err.pushl(__LINE__," tolower test fail for %s", nmtype);
	return 0;
}
static int test_tolower(test_state& err){
	if (test_tolower_t<char>(err)) return 1;
	if (test_tolower_t<wchar_t>(err)) return 1;
	return 0;
}

template <typename TChar> int test_toupper_t(test_state& err) {
	rc_string<TChar> ns = L"-абвгеёжзийклмнопрстуфхцчшщъыьэюя АБВГЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ-abcdef-ABCDEF";
	rc_string<TChar> res = L"-АБВГЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ АБВГЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ-ABCDEF-ABCDEF";
	auto lc = strupper(ns);
	auto nmtype = sizeof(TChar) == 1 ? "char" : "wchar_t";
	if (lc != res) return err.pushl(__LINE__, " test_toupper test fail for %s", nmtype);
	return 0;
}
static int test_toupper(test_state& err) {
	if (test_tolower_t<char>(err)) return 1;
	if (test_tolower_t<wchar_t>(err)) return 1;
	return 0;
}


mDeclareTestProc(test_charcodec, esr_TestIsReady){
//int test_charcodec() {
	if (call_test(test_utf8_wchart, "test_utf8_wchart")) return 1;
	if (call_test(test_wchart_utf8, "test_wchart_utf8")) return 1;
	// upper case
	
	return 0;
}

}};
