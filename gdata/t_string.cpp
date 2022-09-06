#include <iostream>
#include <cassert>
#include "t_string.h"

namespace tbgeneral {


    template <class strt> strt str_replace(strt str, const strt& from, const strt& to) {
        if (from.empty())
            return strt();
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
            // In case 'to' contains 'from', like replacing 'x' with 'yx'
        };
        return str;
    }


/*
template <class strt> strt str_replace(strt str, const strt& from, const strt& to) {
    if (from.empty())
        return strt();
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
        // In case 'to' contains 'from', like replacing 'x' with 'yx'
    };
    return str;
}
*/
}

namespace tbgeneral{


template <typename TChar> rc_string<TChar> string_trim(const rc_string<TChar>& ss) {
        const TChar* s = ss.data(), * e = s + ss.size() , *sfix= s; 
        for (; (s < e) && (isspaceChar(*s)); s++);
        for (; (s < e) && (isspaceChar(e[-1])); e--);
        return ss.slice( s-sfix, e - s);
};

template <typename TChar> rc_string<TChar> string_trim(const TChar * ss , size_t cnt ) {
    rc_string<TChar> v; v.assign_cnstval( ss, cnt );
    auto res = string_trim( v );
    res.MakeSafeStorage();
    return res;
}

rc_string<char> trim(const char* s, size_t cnt){ return string_trim(s,cnt); };
rc_string<wchar_t> trim(const wchar_t* s, size_t cnt){ return string_trim(s, cnt); };
rc_string<char> trim(const rc_string<char>& s) { return string_trim(s); }
rc_string<wchar_t> trim(const rc_string<wchar_t>& s) { return string_trim(s); }

size_t find_char( const char * begin, const char * end , char fc ){
    for ( auto s=begin ; s< end; s++) if (*s==fc) return s-begin;
    return -1;
};
size_t find_char( const wchar_t * begin, const wchar_t * end , char fc ){
    for ( auto s=begin ; s< end; s++) if (*s==fc) return s-begin;
    return -1;
};


int findchars(const rc_string<char>& src, const param_string<char>& find_set){
    char buff[256];
    ZEROMEM(buff);
    for (auto c: find_set.param) buff[c]=1;
    for (auto p=src.begin(), e = src.end(); p<e;p++){
        if (buff[*p]) return static_cast<int>(p-e);
    }
    return -1; 
};





std::basic_ostream<char>& operator << (std::basic_ostream<char>& os, const rc_string<wchar_t>& d){
	return os << rc_string<char>(d);
};
std::basic_ostream<wchar_t>& operator << (std::basic_ostream<wchar_t>& os, const rc_string<char>& d){
	return os << rc_string<wchar_t>(d);
};

template<typename DChar, typename SChar> int push2stream_t(std::basic_ostream<DChar>& os, const SChar* src, size_t cnt) {
    DChar buff[1024];
    size_t readcnt = 0 , rescount=0;
    for (; cnt > 0; cnt -= readcnt) {
        readcnt = 0;
        auto wrcount = sarr_convert(buff, sizeof(buff), src, cnt, readcnt);
        assert( !(wrcount == 0 || readcnt==0 ));
        if (wrcount == 0) break;
        os.write(buff, wrcount);
        rescount += wrcount;
    }
    return static_cast<int>( rescount );
};


int push2stream(std::basic_ostream<char>& os, const wchar_t* d, size_t cnt) {    return push2stream_t(os, d, cnt); };
int push2stream(std::basic_ostream<wchar_t>& os, const char* d, size_t cnt) {    return push2stream_t(os, d, cnt); };

char* memset_char(char* dest, char c, size_t count) {
    return static_cast<char*>( memset(dest, c, count) );
};
wchar_t* memset_char(wchar_t* dest, wchar_t c, size_t count) {
    return wmemset(dest, c, count);
};

template <typename DChar> void nchar2stream_t(std::basic_ostream<DChar>& ss, DChar c, size_t n) {
    DChar buff[64]; size_t szBuff = std::size(buff);
        memset_char(buff, c, std::size(buff));
    for (; n > szBuff; n -= szBuff) ss.write(buff, szBuff);
    ss.write(buff, n);
}
void nchar2stream(std::basic_ostream<char>& ss, char c, size_t n) { nchar2stream_t(ss, c, n); }
void nchar2stream(std::basic_ostream<wchar_t>& ss, wchar_t c, size_t n) { nchar2stream_t(ss, c, n); }


}//namespace tbgeneral
