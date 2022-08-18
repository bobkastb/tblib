// test_tblib.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
//#include "gdata/tbt_strings.h"

#include <iostream>
#include "test_state.h"
#include "gdata/tbt_locale.h"
#pragma comment ( lib , "tblib" )

void test_format();
void test_time();
void test_stdvector_OC();
int test_functions();
namespace tbgeneral {namespace test_ns {
    int test_string();
    int test_darray();
    int test_format();
    int test_stream();
    int test_time();
    int test_log();
    int test_hash();

    int test_ArithmeticCoding();
    int test_binmap();
    int test_blockChifer();
    int test_charcodec();
    int test_communication();
    int test_convert();
    int test_dfa();
    int test_env();
    int test_files();
    int test_json();
    int test_mutex();
    int test_net();
    int test_parse();
    int test_parsecmdline();
    int test_serialize();
    int test_sys_process();
    int test_sys_service();
    int test_tbconf();

typedef int (*test_function_ptr)();
struct test_function_t { test_function_ptr fp; const char* nm; };
#define MF(nm) { nm , #nm }
static test_function_t alltestfunctions[] = { 
    MF(test_darray) , MF(test_string) , MF(test_stream) , MF(test_format) , MF(test_time) , 
    MF(test_log),MF(test_hash)
    //,MF(test_ArithmeticCoding),MF(test_binmap),MF(),MF(),MF()
};

static int call_one_tests(const test_function_t & f) {
    std::cout << "start test <" << f.nm << "> ...";
    auto r= f.fp();
    switch (r) {
    case esr_TestNeedUpdate:  std::cout << "completed but needs improvement!\n"; return 0;
    case esr_TestIsAStub:  std::cout << "this is a stub!...\n"; return 0;
    case 0:  std::cout << "passed\n"; return 0;
    }
    return 1;
}
static int call_one_tests(const tTestProcRegistrator & tr) {
    std::cout << "start test <" << tr.name << "> ...";
    auto r= tr.proc();
    if (r) return 1;
    std::cout << "passed"<< (tr.state== esr_TestNeedUpdate ? " but wait a correction!" : "") << "\n"; 
    return 0;
}

static int call_all_tests( bool stoponerr ) {
    int errs = 0;
    if(0)
    for (auto f : alltestfunctions) {
        auto r = call_one_tests(f);
        if (stoponerr && r) return 1;
        errs += r ? 1 : 0;
    }
    if(1) {
        std::vector<std::string> stublist;
        auto &tests = tTestCollection::gettests();
        for (auto &tr :  tests.regslist ){
            if (tr->state==esr_TestIsAStub) {stublist.push_back(tr->name); continue; }
            auto r = call_one_tests( *tr );
            if (stoponerr && r) return 1;
            errs += r ? 1 : 0;
        }
        if (stublist.size()) {
            std::cout << "Stub list["<<stublist.size()<<"]: ";
            auto sep="";
            for (auto &nm : stublist){
                std::cout << sep << nm;
                sep=" ,";
            }
            std::cout << "\n";
        }
    }
    return errs;
}
static int call_singletest() {
    //return call_one_tests( MF( test_json ) );
    //return call_one_tests( MF( test_time ) );
    //return call_one_tests(MF(test_hash));
    //return test_functions();
    //test_stdvector_OC();
        //test_darray();
    return 0;
}
static int call_test_proc(bool stoponerr) {
    int errs = 0;
    if (call_singletest()) if (stoponerr) return 1;
    errs = call_all_tests(stoponerr);
    if (!errs)
        std::cout << "All tests pased\n\n";
    return errs;
}

}};


int main()
{
    tbgeneral::setconsoleCP_utf8();
    std::cout << "Start testing!\n";
    //auto stoponerr = true;
    auto stoponerr = false;
    tbgeneral::test_ns::call_test_proc(stoponerr);
    //test_format();
    //test_time();
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
