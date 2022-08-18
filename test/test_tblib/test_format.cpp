//#include "gdata/tbt_strings.h"
//#include "gdata/tb_parray.h"
#include "test_darray.h"
static t_operation_statistic opstat_l(_DBG_ARR_START, _DBG_ARR_END, t_operation_statistic::get_DBG_ARR_names());
#define DEBUG_TECH_ARRAY(opcode,mask) opstat_l.onUseOperation(opcode, mask);
#include "gdata/t_format.h"
#include <iostream>
#include <cassert>




#define MSGC_EXTRACTION_THREAD_IS_CREATED "Extraction thread is created"
#define MSGC_EXTRACTION_THREAD_IS_NOT_CREATED "Extraction thread is not created"

struct TMsgConst {
    char msgc_extraction_thread_is_created[sizeof(MSGC_EXTRACTION_THREAD_IS_CREATED)] = MSGC_EXTRACTION_THREAD_IS_CREATED;
    char msgc_extraction_thread_is_not_created[sizeof(MSGC_EXTRACTION_THREAD_IS_NOT_CREATED)] = MSGC_EXTRACTION_THREAD_IS_NOT_CREATED;
};

void test_assign() {
    char data[sizeof(TMsgConst)];
    TMsgConst xx;
    TMsgConst* pp = (TMsgConst*)data; pp = &xx;
    using namespace tbgeneral;
    struct At { int i, j; operator int() const { return 0; } };
    auto & ptid = typeid(At);
    auto y= typeid(int).name();
    bool v[] = { std::is_assignable<int&, int>().value,
    std::is_assignable<int&, At>().value,
    std::is_assignable<int64_t&, At>().value,
    std::is_assignable<double&, At>().value,
    std::is_trivially_assignable<int&, At>().value
    ,std::is_convertible<At,int>::value

    };
    //test_type1<int>();
    //test_type1<void*>();
    //int x = At();
    int z = 0;
}





//---------------------

//---------- CPP 

//--------- TESTS ---------

namespace tbgeneral {   namespace test_ns {
/*
void print_tst(){ std::cout <<  "\n"; }
template <typename First, typename... Rest> void print_tst(const First& first, const Rest&... rest) {
        std::cout << first << ", ";
        print_tst(rest...); // recursive call using pack expansion syntax
}
*/


//---------------


#pragma warning( disable : 4476 )
#pragma warning( disable : 4474 )

template<typename VType> struct t_test_data {
    const char* fmt;
    VType data;
    const char* result;
};
struct test_ctx {
    test_state& err; 
    rc_string<char>& res;
    bool usestd_printf;
};
template<typename VType> const char* do_std_printf(const char * fmt , const VType & d ) {
    static char buff[4*1024];
    auto r = snprintf(buff, sizeof(buff), fmt , d );
    return buff;
}

template<typename VType> int test_cmp(test_ctx& ctx, const t_test_data<VType>& t , int i=0 , int lineno=0 ) {
    ctx.res.resize(0);
    format_to(ctx.res, t.fmt, t.data);
    if (ctx.res != t.result) return ctx.err.pushf("line:(%d,%d) cmp error %s!=%s", i, lineno, ctx.res.c_str(),t.result);
    if (ctx.usestd_printf) {
        auto stdres = do_std_printf(t.fmt, t.data);
        if (ctx.res != stdres) return ctx.err.pushf("line:(%d,%d) cmp stdprintf error %s!=%s", i, lineno, ctx.res.c_str(), stdres);
    }
    return 0;
}

template<typename VType> int test_cmp(test_ctx & ctx, int lineno,const t_test_data<VType>* arr, size_t cnt) {
    for (size_t i = 0; i < cnt ; i++) 
        if (test_cmp(ctx, arr[i], (int) i,lineno)) return 1;
    return 0;
}


template<typename FType> int test_format_float() {
    FType fa[] = { FType(23456789012334.468773346) , FType(233.444E+20)  , FType(233.444E-20) };
    auto res = format("  ==%f==%.6e==%f==%10.3e==\n", fa[0], fa[0], fa[1], fa[1]);
    return 0;
}

template <typename TChar, typename... Rest> void format_toR(rc_string<TChar>& res, const TChar* fmt, const Rest&... rest) {
    res.resize(0);
    return format_to(res, str_aslocal(fmt), rest...);
}

int test_format_int(test_state& err) {
    rc_string<char> res;   res.reserve(4 * 1024);
    test_ctx ctx = { err, res , true };
    //format_to(res, "%05d--%5d--%-5d--%+5d", 55,55,55,55);
    int mstst[] = { opstat_l._count_op_alloc , opstat_l._count_op_free };

    format_toR(res,"--%x--%x--%x--%llx--==", 0x55, 0xAA55, 0x55AA55AA, 0xBB7755AA55AA);
    if (res != "--55--aa55--55aa55aa--bb7755aa55aa--==") return err.pushl(__LINE__, "err cmpstr");
    format_toR(res, "--%X--%X--%X--%X--==", 0xCC, 0xAABB, 0xAABBCCDD, 0xAABBCCDDEEFF);
    if (res != "--CC--AABB--AABBCCDD--AABBCCDDEEFF--==") return err.pushl(__LINE__, "err cmpstr");
    format_toR(res, "--%#x--%#x--%#X--%#X--==", 0xCC, 0xAABB, 0xAABBCCDD, 0xAABBCCDDEEFF);
    if (res != "--0xcc--0xaabb--0XAABBCCDD--0XAABBCCDDEEFF--==") return err.pushl(__LINE__, "err cmpstr");
    format_toR(res,"--%05x--%5x--%-5x--", 0x55, 0x55, 0x55 );
    if (res != "--00055--   55--55   --") return err.pushl(__LINE__, "err cmpstr");
    format_toR(res,"--%05d--%5d--%-5d--%+5d--", 55, 55, 55,55);
    if (res != "--00055--   55--55   --  +55--") return err.pushl(__LINE__, "err cmpstr");
    format_toR(res, "--%u--%5u--%-5u--%10u--", -55, -55, -55, -55);
    if (res != "--4294967241--4294967241--4294967241--4294967241--") return err.pushl(__LINE__, "err cmpstr");

    int msten[] = { opstat_l._count_op_alloc , opstat_l._count_op_free };
    if (msten[0] != mstst[0] || msten[1] != mstst[1])
        err.pushl(__LINE__, "invalid mem operation count");

    return 0;
}

int test_format_str(test_state& err) {
    rc_string<char> res;   res.reserve(4 * 1024);
    test_ctx ctx = { err, res , true };
    int mstst[] = { opstat_l._count_op_alloc , opstat_l._count_op_free };

    t_test_data<const char*> arr1[] = {
        {"+%s+","rr","+rr+"},{"+%5s+","rr","+   rr+"},{"+%-5s+","rr","+rr   +"}
        ,{"+%s+","rr-rr-rr","+rr-rr-rr+"}
    };
    //if (test_cmp(ctx,arr1[1])) return 1;
    if (test_cmp(ctx, __LINE__, arr1, std::size(arr1))) return 1;

    ctx.usestd_printf = false;
    t_test_data<const wchar_t*> arr2[] = {
        {"+%s+",L"rr","+rr+"},{"+%5s+",L"rr","+   rr+"},{"+%-5s+",L"rr","+rr   +"}
        ,{"+%s+",L"яя",u8"+яя+"},{"+%5s+",L"яя",u8"+   яя+"},{"+%-5s+",L"яя",u8"+яя   +"}
        ,{"+%s+",L"rr-rr-rr","+rr-rr-rr+"}
    };
    //if (test_cmp(err,res,arr2[4])) return 1;
    if (test_cmp(ctx, __LINE__, arr2, std::size(arr2))) return 1;

    int msten[] = { opstat_l._count_op_alloc , opstat_l._count_op_free };
    if (msten[0] != mstst[0] || msten[1] != mstst[1])
        err.pushl(__LINE__, "invalid mem operation count");
    return 0;
}

int test_format_float(test_state& err) {
    const float funper = float( 100.0 / 3);
    const double dunper = 100.0 / 3;
    rc_string<char> res;  res.reserve(4 * 1024);
    test_ctx ctx = { err, res , true };

    int mstst[] = { opstat_l._count_op_alloc , opstat_l._count_op_free };

    t_test_data<float> arr1[] = {
        {"+%f+",55,"+55.000000+"},{"+%12f+",55,"+   55.000000+"},{"+%-12f+",55,"+55.000000   +"}
        ,{"+%f+",funper,"+33.333332+"},{"+%-5f+",funper,"+33.333332+"}
    };
    t_test_data<double> arr2[] = {
        {"+%e+",55,"+5.500000e+01+"},{"+%13e+",55,"+ 5.500000e+01+"},{"+%-13e+",55,"+5.500000e+01 +"}
        ,{"+%e+",dunper,"+3.333333e+01+"},{"+%10f+",dunper,"+ 33.333333+"}
    };
   // if (test_cmp(ctx,arr2[4])) return 1;
    if (test_cmp(ctx, __LINE__, arr1, std::size(arr1))) return 1;
    if (test_cmp(ctx, __LINE__, arr2, std::size(arr2))) return 1;

    int msten[] = { opstat_l._count_op_alloc , opstat_l._count_op_free };
    if (msten[0] != mstst[0] || msten[1] != mstst[1])
        err.pushl(__LINE__, "invalid mem operation count");

    return 0;
};

int test_format_b() {
    //std::basic_ostream<char> *tt;
    //print_tst(3,4,5,6);

    //test_std_printf();
    rc_string<char> res;
    res = format("  --%x--%x--%x--%llx--==\n", 0x55, 0xAA55, 0x55AA55AA, 0xBB7755AA55AA);

    test_format_float<float>();
    test_format_float<double>();
    res = format("  --%s--%s-- ==\n", L"rrr", L"aaa");
    res = format("  --%s-- ==\n", L"яяяяяяя");
    res = format("  --%5s--%-5s-- ==\n", "rrr", "aaa");
    res = format("  %d -- %d -- %d -- %f -- ==\n", 1, 2, 3);
    return 0;
}


static void val11(const param_string<char>& p) {
    auto i = p.param;
}

static int test_std_printf() {
    char buff[1024];
    auto r = snprintf(buff, sizeof(buff), "%10f", float(55) );
    char buff1[] = "333";
    stringA sa(buff1);
    val11("eee");
    val11(buff1);
    val11(sa);
    return 0;
}

mDeclareTestProc(test_format,esr_TestIsReady){
//int test_format() {
    test_std_printf();
    if (call_test(test_format_str, "test_format_str")) return 1;
    if (call_test(test_format_int, "test_format_int")) return 1;
    if (call_test(test_format_float, "test_format_float")) return 1;

    //test_format_b();
    return 0;
}

}};