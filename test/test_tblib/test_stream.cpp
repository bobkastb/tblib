#include <sstream>
#include <functional>
#include "test_state.h"
#include "gdata/t_sstream.h"
#include "gdata/t_string.h"




//#include <format>
namespace tbgeneral {namespace test_ns {
    using TChar = char;

int test_stdstream(){
    std::ostringstream ss;
    ss << "Operation with id = " << 44 << " failed, because data1 (" << 55 << ") is incompatible with data2 (" << 53 << ")";
    auto s= ss.str();
    return 0;
}
int test_my_stream() { // basic_ostream<char, char_traits<char>>;

    {   char buff[4*1024];
        orcstringstream ss;
        ss.setstart_localbuff(buff, sizeof(buff));
        auto datac1 = "test1\n";
        ss << datac1;
        ss<< "r " << 50 <<"\n"; 
        ss<< "x " << 51 << "\n";
        auto tstr = ss.str();
        //std::cout << tstr;

    }
    basic_rc_stringbuf<char> bf;
    {
        std::ostream strm(&bf);
        strm << "r";

        strm << "end";
    }

    return 0;
}

int test_buff_stream(){
    stringA res;   stringA* pres = &res;
    int cntflush=0;
    auto f = [&](const char* b,size_t sz){ 
        pres->append(b,sz); cntflush++; };
    limit_orcstringstream<char> os( f , 70 );
    for (int i=0;i<21;i++) {
        char b[]="0123456789";
        os << b;
    }
    os.flush();
    if (cntflush!=3) return 1;

    return 0;
}

//int test_stream(){
mDeclareTestProc(test_stream,esr_TestIsReady){
    //test_stdstream();
    limit_rc_stringbuf<char> sb; 
    test_buff_stream();
    test_my_stream();
    return 0;
}

}}