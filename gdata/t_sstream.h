#pragma once

#include "gdata/t_string.h"

#include <iostream>
#include <functional>


namespace tbgeneral {


    // basic_rc_stringbuf
    template <typename TChar> class basic_rc_stringbuf : public std::basic_streambuf<TChar> { 
        using _MyBase = std::basic_streambuf<TChar>;
        using int_type = typename _MyBase::int_type;
        using traits_type=typename _MyBase::traits_type;
    public:
        rc_string<TChar> buff;
        rc_string<TChar> str() { return buff; };
        void str(const rc_string<TChar>& newbuf) { buff = newbuf; };
    protected:
        virtual std::streamsize xsputn(const TChar* s, std::streamsize n) {
            buff.append(s, static_cast<size_t>(n));
            return n;
        };
        virtual int_type overflow(int_type c = traits_type::eof()) {
            if (traits_type::eq_int_type(traits_type::eof(), c)) return traits_type::not_eof(c);
            TChar cc = traits_type::to_char_type(c);
            buff.append(&cc, 1);
            return c;
        };
    }; // basic_rc_stringbuf



    template <typename TChar> class basic_orcstringstream : public std::basic_ostream<TChar> {
        using _MyBase = std::basic_ostream<TChar>;
        void* external_storage=0;
    public:
        basic_rc_stringbuf<TChar> strbuff;
        basic_orcstringstream() : _MyBase(&strbuff) { }
        basic_orcstringstream(void* localbuff, size_t capa) :_MyBase(&strbuff) {
            external_storage = localbuff;
            setstart_localbuff(localbuff, capa);
        }
        // Получить строку с содержимым потока в безопасном варианте 
        rc_string<TChar> str() {
            auto res = strbuff.str();
            res.free_external_storage(external_storage);
            return res;
        };
        const rc_string<TChar>& nosecstr() {
            return strbuff.buff;
        };
        void str(const rc_string<TChar>& newbuf) { strbuff.str(newbuf); };

        // Установка внешнего буфера как начальной памяти потока
        void setstart_localbuff(void* localbuff, size_t capa) {
            external_storage = localbuff;
            strbuff.buff.assign_storage(localbuff, capa);
            //rc_string<TChar>
        }
    }; // basic_orcstringstream


    // Буффер с ограничением размера. При переполнении вызывается внешний flush
    // сделан только для типа char. Если размер буфера = 0 , то будет простое перенаправление.
    template <typename TChar> class limit_rc_stringbuf : public std::basic_streambuf<TChar> {
        using _MyBase = std::basic_streambuf<TChar>;
        using int_type = typename _MyBase::int_type;
        using traits_type=typename _MyBase::traits_type;
    public:
        //using ft_callback = std::function<void(const char*, size_t)>;
        using char_type = TChar;
        typedef std::function<void(const TChar*, size_t)>  ft_callback;
        rc_string<TChar> buff;
        ft_callback flush_dest;
        void flush();
        void make(ft_callback f,  size_t buffsize , void* localbuff=0) {
            flush_dest = f;
            if (!localbuff) { if (buffsize > 1) buff.reserve(buffsize);  
            } else buff.assign_storage(localbuff, buffsize); }
    protected:
        void write_proc(const TChar* s, size_t n);
        virtual int sync() { flush(); return 0; }
        virtual std::streamsize xsputn(const TChar* s, std::streamsize n);
        virtual int_type overflow(int_type c = traits_type::eof());
        virtual std::streampos seekoff(std::streamoff off, std::ios_base::seekdir way, std::ios_base::openmode which ){
            return buff.size();
        };
    }; // limit_rc_stringbuf

    // Поток с буффером ограниченного размера с перенаправлением при переполнении. При переполнении вызывается внешний flush, сделан только для типа char
    template <typename TChar> class limit_orcstringstream : public std::basic_ostream<TChar> {
        using _MyBase = std::basic_ostream<TChar>;
    public:
        using ft_callback = typename limit_rc_stringbuf<TChar>::ft_callback;
        limit_rc_stringbuf<TChar> strbuff;
        limit_orcstringstream() :_MyBase(&strbuff) { }
        limit_orcstringstream(ft_callback f,  size_t capa , void* localbuff=0 ) :_MyBase(&strbuff) {
            strbuff.make(f,  capa , localbuff);
        }
        void make(ft_callback f,  size_t capa , void* localbuff=0){ strbuff.make(f, capa, localbuff);  }
        void flushbuff() { strbuff.flush(); }
    }; // limit_orcstringstream



    using orcstringstream = basic_orcstringstream<char>;
    template<typename TChar> int push2stream(std::basic_ostream<TChar>& os, const TChar* d, size_t cnt) { 
        os.write(d, cnt);  return static_cast<int>(cnt); };
    int push2stream(std::basic_ostream<char>& os, const wchar_t* d, size_t cnt);
    int push2stream(std::basic_ostream<wchar_t>& os, const char* d, size_t cnt);

    void nchar2stream(std::basic_ostream<char>& ss, char c, size_t n);
    void nchar2stream(std::basic_ostream<wchar_t>& ss, wchar_t c, size_t n);


};
#pragma once
