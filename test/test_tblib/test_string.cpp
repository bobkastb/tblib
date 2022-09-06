#include "test_darray.h"


#include "gdata/t_string.h"
enum {
	eStrResetOnZero = tbgeneral::rc_string<char>::eRESETonRESIZE0
	, eArrResetOnZero = tbgeneral::rc_array<char>::eRESETonRESIZE0
};


namespace tbgeneral {namespace test_ns {



const char Special0= 1;
using TChar = char;

int cmp_chardata(const rc_string<TChar>& s, const TChar* r){
	TChar buff[1024]; auto sz =  std::min(strlen_s(r),sizeof(buff)-1);
	if (sz!=s.size()) return 1;
	memcpy( buff , r , sz );
	for (size_t i=0;i<sz;i++) if (buff[i]== Special0) buff[i]=0;
	return strncmp( s.data() , buff , sz );
};

template <typename TChar> size_t calc_str_pos(const rc_string<TChar> & vec) {
	return vec.sdata ? vec.sdata - vec.fstorage->fdata : 0;
}
//test_operations_loc_t
int runtest_datachars(int lineno, int index, test_state& err, test_operations_loc_t operloc , size_t flags, const test_datachars_tt<TChar>& test) {
	rc_string<TChar> vec, vec2;
	if (flags & efRESERVE) vec.reserve(128);
	if (flags & efCnstSeg)
		vec.assign_cnstval(test.source, strlen_s(test.source));
	else vec.assign(test.source, strlen_s(test.source));
	if (flags & efREFS) vec2 = vec;
	test_operations_loc_t sl = { 0,0 };
	if (test.preSlice.size()) { sl = test.preSlice[0]; vec = vec.slice(sl.start, sl.size); }
	auto oldst = vec.getstorage();
	auto oldstpos = calc_str_pos(vec);
	switch (test.operationcode) {
	case eop_Reserve: vec.reserve(operloc.size); break;
	case eop_Resize: vec.resize(operloc.size); break;
	case eop_Append: vec.append(test.opdata);  break;
	case eop_Insert: vec.insert(operloc.start, test.opdata); break;
	case eop_Erase: vec.erase(operloc.start, operloc.size); break;
	case eop_InsertIntersect: vec.insert(operloc.start, vec.cdata() + operloc.start, operloc.size); break;
	case eop_AssignIntersect: vec.assign(vec.cdata()+operloc.start, operloc.size); break;
	case eop_MakeUnique: vec.makeunique(); break;
	case eop_Trim : vec= trim(vec); break;
	}
	const char* errtr = 0;
	auto r = cmp_chardata(vec, test.opresult); if (r) errtr = "";
	if (flags & efTestMemRes) {
		auto currpos = calc_str_pos(vec);
		if ((oldst != vec.fstorage) != ((flags & efwRealloc)!=0)) errtr = "!realloc error";
		if ((0 == vec.fstorage) != ((flags & efwStorageZero) != 0)) errtr = "!storage non zero";
		if ((oldstpos != currpos) != ((flags & efwChangeStart) != 0)) errtr = "!change start pos";
	}
	if (errtr) {
		std::string resstr(vec.data(), vec.size());
		std::string opinf = get_operation_presentation(test.operationcode, operloc, test.opdata);
		auto slinf = format_ss(" slice=%d[%d]", sl.start, sl.size);
		return err.pushf("line(%d,%d) %s->%s!=%s ctx: %s%s%s %s", index, lineno, opinf.c_str(), resstr.c_str(), test.opresult
			, (flags & efREFS ? " REFS" : ""), (flags & efRESERVE ? " RESERVE" : ""), (test.preSlice.size() ? slinf.c_str() : "")
			, errtr
		);
	}
	return 0;
}
int runtest_datachars(int lineno,int index, test_state& err, size_t flags, const test_datachars_tt<TChar> & test ) {
	for ( auto operloc : test.operloc) {
		auto r=runtest_datachars(lineno, index, err, operloc, flags, test);
		if (r) return r;
	}
	return 0;
}

int runtest_datachars(int lineno, test_state& err, bool forAllFlags, const std::vector<test_datachars_tt<TChar>>& ctests) {
	std::vector<size_t> flags_list = { 0 };
	if (forAllFlags) flags_list = { 0, 1 , 2, 3, 4, 5, 6, 7 };
	for (auto flags : flags_list)
		for (size_t i = 0; i < ctests.size(); i++) {
			if (runtest_datachars(lineno, (int)i, err, flags, ctests[i])) return 1;
		}
	return 0;
}
int runtest_datachars_mempos(int lineno,int index, test_state& err,  const test_datachars_tt<TChar>& test) {
	auto fl = test.oflags | efTestMemRes;
	return runtest_datachars(lineno, index, err, fl, test);
}
int runtest_datachars_mempos(int lineno, test_state& err,  const std::vector<test_datachars_tt<TChar>>& ctests) {
	for (size_t i = 0; i < ctests.size(); i++) {
		auto fl = ctests[i].oflags | efTestMemRes;
		if (runtest_datachars(lineno, (int) i, err, fl , ctests[i])) return 1;
	}
	return 0;
}


static int test_t_move(test_state& err){
	return 0;
}

static int test_data_str_t(test_state& err) {
	std::vector<test_datachars_tt<TChar>> tests;
	auto defsrc = "0123456789AB";
	auto trmsrc=" \n	0123456789AB	\n ";
	enum {szDD=12};
	// efwChangeStart=0x10 , efwRealloc = 0x20 , efwStorageZero= 0x40
	tests = {
{ eop_Resize , defsrc ,  {} , "" , {{ 0,szDD + 3 }} , "0123456789AB\1\1\1" , efwRealloc }
,{ eop_Resize , defsrc ,  {} , "" , {{ 0,szDD - 3 }} , "012345678" , 0 }
,{ eop_Resize , defsrc ,  {} , "" , {{ 0,1 }} , "0" , 0 }
,{ eop_Resize , defsrc ,  {} , "" , {{ 0,0 }} , ""  , (eStrResetOnZero ? efwStorageZero| efwRealloc:0) }
,{ eop_Reserve , defsrc ,  {} , "" , {{ 0,szDD+12 }} , "0123456789AB" , efwRealloc }

,{ eop_Reserve , defsrc ,  {} , "" , {{ 0,0 }} , "0123456789AB" , 0 }
,{ eop_Insert , defsrc ,  {} , "adbce" , {{ 0,0 }} , "adbce0123456789AB" , efwRealloc }
,{ eop_Insert , defsrc ,  {} , "adbce" , {{ szDD,0 }} , "0123456789ABadbce" , efwRealloc }
,{ eop_Insert , defsrc ,  {} , "adbce" , {{ szDD+2,0 }} , "0123456789ABadbce" , efwRealloc }
,{ eop_Insert , defsrc ,  {} , "adbce" , {{ 3 ,0 }} , "012adbce3456789AB" , efwRealloc }
//10
,{ eop_Insert , defsrc ,  {} , "adbce" , {{ 10 ,0 }} , "0123456789adbceAB" , efwRealloc }
,{ eop_Append , defsrc ,  {} , "adbce" , {{ 0 ,0 }} , "0123456789ABadbce"  , efwRealloc}
,{ eop_InsertIntersect , defsrc ,  {} , "" , {{ 0 ,12 }} , "0123456789AB0123456789AB" , efwRealloc}
,{ eop_InsertIntersect , defsrc ,  {} , "" , {{ 2 ,10 }} , "0123456789AB23456789AB" , efwRealloc}
,{ eop_InsertIntersect , defsrc ,  {} , "" , {{ 11 ,1 }} , "0123456789ABB" , efwRealloc}

,{ eop_AssignIntersect , defsrc ,  {} , "" , {{ 0 ,13 }} , "0123456789AB\1", efwRealloc }
,{ eop_AssignIntersect , defsrc ,  {} , "" , {{ 0 ,2 }} , "01" , efwRealloc}
,{ eop_AssignIntersect , defsrc ,  {} , "" , {{ 0 ,10 }} , "0123456789" , efwRealloc }
,{ eop_AssignIntersect , defsrc ,  {} , "" , {{ 5 ,szDD-5 }} , "56789AB" ,efwRealloc }
,{ eop_MakeUnique , defsrc ,  {} , "" , {{ 0,0 }} , "0123456789AB" , 0}
//20
,{ eop_Erase , defsrc ,  {} , "" , {{ 0 , 2 }} , "23456789AB" ,0 }
,{ eop_Erase , defsrc ,  {} , "" , {{ 10 , 2 }} , "0123456789" ,0}
,{ eop_Erase , defsrc ,  {} , "" , {{ 1 , 8 }} , "09AB" ,0}
,{ eop_Trim , trmsrc ,  {} , "" , {{0,0}} , "0123456789AB" , efwChangeStart }


	};
	//if (runtest_datachars_mempos(__LINE__, 0, err, tests[23])) return 1;
	//if ( runtest_datachars(__LINE__, 0, err, 0, tests[15])) return 1;
	if (runtest_datachars(__LINE__, err, true, tests)) return 1;
	if ( runtest_datachars_mempos(__LINE__, err,  tests)) return 1;
	return 0;
}

template <typename C> int cmptestEq( int lineno, test_state& err , const rc_string<C>& l, const C* r) {
	if (l == r) return 0;
	return err.pushl(lineno, "error cmp");
}
//#include <sstream>

static void test_noexplicit(const rc_string<char>& s) {}

static int test_convert_str(test_state& err){
	auto std_as = std::string("rrr");
	auto std_ws = std::wstring(L"www");
	#define Atest(cstr) cmptestEq(__LINE__,err, as,cstr)
#define Wtest(cstr) cmptestEq(__LINE__,err, ws,L##cstr)
	rc_string<char> as(std_as); if (Atest("rrr")) return 1;
	as= rc_string<char>(std_ws); if (Atest("www")) return 1;
	as = "rrr"; if (Atest("rrr")) return 1;
	as = L"www"; if (Atest("www")) return 1;
	as = std_as; if (Atest("rrr")) return 1;
	as = std_ws; if (Atest("www")) return 1;
	as += std_as; if (Atest("wwwrrr")) return 1;
	as += std_ws; if (Atest("wwwrrrwww")) return 1;
	rc_string<wchar_t> ws(std_as);	
	if (Wtest("rrr")) return 1;
	ws = rc_string<wchar_t>(std_ws);	if (Wtest("www")) return 1;
	ws = "rrr";		if (Wtest("rrr")) return 1;
	ws = L"www";	if (Wtest("www")) return 1;
	ws = std_as;	if (Wtest("rrr")) return 1;
	ws = std_ws;	if (Wtest("www")) return 1;
	ws += std_as;	if (Wtest("wwwrrr")) return 1;
	ws += std_ws;	if (Wtest("wwwrrrwww")) return 1;
	as = "rrr";
	ws = rc_string<wchar_t>(as);	if (Wtest("rrr")) return 1;
	ws = "www";
	as = rc_string<char>(ws);		if (Atest("www")) return 1;
	as = ws;		if (Atest("www")) return 1;
	ws = as;		if (Wtest("www")) return 1;
	as += ws;		if (Atest("wwwwww")) return 1;
	as += as;		if (Atest("wwwwwwwwwwww")) return 1;
	ws +=as;		if (Wtest("wwwwwwwwwwwwwww")) return 1;
	ws += ws;		if (Wtest("wwwwwwwwwwwwwwwwwwwwwwwwwwwwww")) return 1;
	as = as + ws + std_ws + std_as;
	as += "rrr";
	as += L"rrr";
	auto uu = as + "rrr" + as;
	as = as + L"rrr";
	as.clear();
	test_noexplicit("ddd");

	//wide_to_utf8(L"rrrr", as);
	//char c1= as[0];
	//auto c = as.cdata();

	//std::stringstream ff;
	//std::basic_stringstream<char> ff;
	//std::ostringstream ff;
	return 0;
}

static int test_init_str(test_state& err) {
	rc_string<TChar> x = "rrr";
	std::string f = x;
	return 0;
}

static int test_move_str(test_state& err) {
		//if (test_t_move<Bin_st>(err)) return 1;
		//if (test_t_move<char>(err)) return 1;
		//if (test_t_move<wchar_t>(err)) return 1;
		//if (test_t_move_slice<nMVst>(err)) return 1;
		//if (test_t_move_slice<MVst>(err)) return 1;

		return 0;
}
static int test_data_str(test_state& err) {
	if (test_data_str_t(err)) return 1;
	return 0;
}

static int test_any(test_state& err) {
	rc_string<char> s;
	s << "rr";
	std::map<rc_string<char>,int> tt;
	auto s1 = stringA(u8"янкель и фоткель");
	auto s2 = s1.slice( 12 );
	auto s3=  s1.slice( 0, 12);
	auto d1 = rc_array<stringA>{ s1, s2 ,s3 , "--e3--" , "--e4--"};
	auto d2 = d1.slice( 1, 2);
	auto di1 = rc_array<int>{ 0,1,2,3,4,5,6,1 };
	auto di2 = di1.slice( 1, 2);
	{ char buff[200];
		auto s=s1.tocstr(buff);
		//char* rr = (char*)"eeee";rr[0] = 0;
	}
	return 0;
}



mDeclareTestProc(test_string,esr_TestIsReady){
//int test_string() {
	
	if (call_test(test_init_str, "test_init_str")) return 1;
	if (call_test(test_move_str, "test_move_str")) return 1;
	if (call_test(test_data_str, "test_data_str")) return 1;
	if (call_test(test_convert_str, "test_convert_str")) return 1;
	if (call_test(test_any, "test_any")) return 1;

	return 0;
}

}};

