#pragma once

#include "gdata/t_string.h"

namespace tbgeneral {


enum { eUTF8=0x75746638 //'utf8'
};

int  get_char_codepage_type( );
int  set_char_codepage_type( int newt);


bool utf8_Validate(const stringA & src , stringA * validpart=0);


stringW utf8_to_wide( const char* s , size_t size );
stringW utf8_to_wide( const char* s );
stringW utf8_to_wide( const stringA & s );
size_t utf8_to_wide( stringW & dest  , const stringA & src );
size_t utf8_to_wide_add(stringW& dest, const char* src , size_t size );

// возвращает количество записанных байт , cntread_wc - количество прочитанных символов
stringA wide_to_utf8( const wchar* s , size_t size );
stringA wide_to_utf8( const wchar* s );
inline stringA wide_to_utf8( const stringW & s ) {return wide_to_utf8(s.cbegin(), s.size()); };

char* wide_to_utf8(  char* buff , uint buffsz , wchar w  );
size_t wide_to_utf8(  stringA & buff , wchar w   );
size_t wide_to_utf8( stringA & buff , const wchar *w , size_t cnt  );
//uint wide_to_utf8(  const wchar * w , uint wsz , stringA & buff );

// добавить перекодированное в буффер и вернуть слайс на результат
stringA wide_to_utf8_add( stringA & buff , const wchar *w , size_t cnt  );
stringA wide_to_utf8_add(  stringA & buff , wchar w   );
size_t wide_to_utf8_b(  const wchar * w , size_t wsz , size_t * cntread_wc , char* buff , size_t csz );

int encode_str_to_consoleCP( char * dest , size_t dsz , const char * src);
stringA encode_str_to_consoleCP( const char * s , size_t sz );
stringA encode_str_to_consoleCP( stringA  s );
void con_print( const char * s  );
void con_printf( const char * fmt , ... );
//void vcon_printf( const char * fmt , va_list argptr );
//void con_printf( const char * fmt , ... );
//void con_print( const wchar * s  );
//void vcon_printf( const wchar * fmt , va_list argptr  );
//void con_printf( const wchar * fmt , ... );
int setconsoleCP_utf8();


size_t deflocale_to_utf16(wchar_t* ws, size_t wsize, const char* src, size_t src_size);
size_t utf16_to_deflocale(char* ws, size_t csize, const wchar_t* src, size_t src_size);

};


//#include <wctype.h>
//#include <cwctype>
//#include <clocale>
#include <codecvt>


namespace tbgeneral {

	struct codec_utf8_utf16 {
		std::codecvt_utf8_utf16<wchar_t> codec;
		std::mbstate_t mbs;
		int in(const char * cs, const char * ce, const char *  & curr, wchar_t * ws, wchar_t * we, wchar_t * & wc) {
			return codec.in(mbs, cs, ce, curr, ws, we, wc);
		};
		int out(const wchar_t * ws, const wchar_t * we, const wchar_t * & wc, char * cs, char * ce, char *  & cc) {
			return codec.out(mbs, ws, we, wc, cs, ce, cc);
		};
		codec_utf8_utf16() { mbs = std::mbstate_t(); }
	};


	struct codec_case_utf16 {
	private:
		const std::ctype<wchar_t> *facet_f;
	public:
		codec_case_utf16() {
			facet_f = &std::use_facet<std::ctype<wchar_t>>(std::locale());
		}
		size_t f_tolower(wchar_t * cs, wchar_t * ce); // return count chars, change buff cs
		size_t f_toupper(wchar_t * cs, wchar_t * ce); // return count chars, change buff cs
		int icmp(const wchar_t * cl, const wchar_t * cr, size_t szl, size_t szr);
		size_t prepareforHashIcase(void* rbuff, size_t rbsz, const wchar_t * cs, const wchar_t * ce, const wchar_t * &cc);
	};
	struct codec_case_utf8 {
	private:
		codec_case_utf16 cc16;
		codec_utf8_utf16 codec8_16;
		size_t f_ul(int opcode, char * cs, char * ce); // return count chars(bytes), change buff cs
	public:
		size_t f_tolower(char * cs, char * ce) { return f_ul(0, cs, ce); }; // return count chars(bytes) , change buff cs
		size_t f_toupper(char * cs, char * ce) { return f_ul(1, cs, ce); }; // return count chars(bytes), change buff cs
		int icmp(const char * cl, const char * cr, size_t szl , size_t szr );
		size_t prepareforHashIcase(void* rbuff, size_t rbsz, const char * cs, const char * ce, const char * &cc);
	};
	template <typename TChar> struct codec_case_t {};
	template <> struct codec_case_t<char> { codec_case_utf8 caser; };
	template <> struct codec_case_t<wchar_t> { codec_case_utf16 caser; };

};