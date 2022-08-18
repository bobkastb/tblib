//#include "tbt_strings.h"
//#include "gdata/gen_tools.h"
//#include "gdata/tb_algo.h"

#include "tbt_locale.h"
//#include <vector>
#include <codecvt>
#include <locale>
#include <iostream>
//#include <string.h>
//#include <wchar.h>




//warning_PUSH();
//#pragma warning( push )
//#pragma warning( disable : 4309 )

//#pragma warning( pop )

namespace tbgeneral {

size_t codec_case_utf16::f_tolower(wchar_t * cs, wchar_t * ce) {
	auto r = facet_f->tolower(cs, ce);
	return ce - r;
};
size_t codec_case_utf16::f_toupper(wchar_t * cs, wchar_t * ce) {
	auto r = facet_f->toupper(cs, ce);
	return ce - r;
};

int codec_case_utf16::icmp(const wchar_t * cl, const wchar_t * cr, size_t szl, size_t szr) {
	enum { BufLen = 1024, ESize = sizeof(wchar_t) };
	wchar_t buff_l[BufLen], buff_r[BufLen];
	auto sz = std::min(szl, szr);
	for (; sz != 0;) {
		auto cnt = std::min<size_t>(sz, BufLen);
		memcpy(buff_l, cl, cnt*ESize);
		memcpy(buff_r, cr, cnt*ESize);
		facet_f->toupper(buff_l, buff_l + cnt);
		facet_f->toupper(buff_r, buff_r + cnt);
		auto r = memcmp(buff_l, buff_r, cnt*ESize);
		if (r != 0) return r;
		sz -= cnt; cl += cnt; cr += cnt;
	}
	return compare(szl, szr);
};
size_t codec_case_utf16::prepareforHashIcase(void* rbuff, size_t rbsz, const wchar_t * cs, const wchar_t * ce, const wchar_t * &cc) {
	enum { ESize = sizeof(*cs) };
	auto wb = (wchar_t*)rbuff; rbsz = rbsz - (rbsz%ESize);
	auto sz = std::min<size_t>(ce - cs, rbsz);
	memcpy(rbuff, cs, sz*ESize);
	this->f_toupper(wb, wb + sz);
	cc = cs + sz;
	return sz*ESize;
};

size_t codec_case_utf8::f_ul(int opcode, char * cs, char * ce) {
	enum { BufLen = 2048 };
	wchar_t buff[BufLen], *wc; const char* cc; const wchar_t* wwc; char* ccc;
	size_t fullsz = 0;
	for (; cs < ce;) {
		auto r = codec8_16.in(cs, ce, cc, buff, buff + BufLen, wc);
		auto cntc = wc - buff; if (cntc == 0) break;
		fullsz += cc - cs;
		auto szul = (opcode ? cc16.f_toupper(buff, wc) : cc16.f_tolower(buff, wc));
		r = codec8_16.out(buff, wc, wwc, cs, ce, ccc);
		if (ccc != cc) throw 1;
		cs = ccc;
	}
	return fullsz;
};
int codec_case_utf8::icmp(const char * cl, const char * cr, size_t szl , size_t szr ) {
	enum { BufLen = 1024 };
	wchar_t buff_l[BufLen], buff_r[BufLen], *wcl, *wcr;
	const char* ccl, *ccr;
	auto cle = cl + szl, cre = cr + szr;

	for (;;) {
		size_t cnts[] = { size_t(cle - cl), size_t(cre - cr) };
		if (cnts[0] == 0 || cnts[1] == 0)
			return compare(cnts[0], cnts[1]);
		auto rl = codec8_16.in(cl, cle, ccl, buff_l, buff_l + BufLen, wcl);
		auto rr = codec8_16.in(cr, cre, ccr, buff_r, buff_r + BufLen, wcr);
		size_t cnta[] = { size_t(wcl - buff_l) , size_t(wcr - buff_r) };
		if (cnta[0]==0 || cnta[1]==0)
			return compare(cnta[0], cnta[1]);
		cc16.f_toupper(buff_l, wcl);
		cc16.f_toupper(buff_r, wcr);
		int rmc = memcmp(buff_l, buff_r, sizeof(*buff_l)*std::min(cnta[0], cnta[1]));
		if (rmc != 0) return rmc;
		else if (cnta[0] != cnta[1])
			return compare(cnta[0], cnta[1]);
		cl = ccl; cr = ccr;
	}
	return 0;
};
size_t codec_case_utf8::prepareforHashIcase(void* rbuff, size_t rbsz, const char * cs, const char * ce, const char * &cc) {
	enum { ESize = sizeof(wchar_t) };
	auto wb = (wchar_t*)rbuff, wc = wb;
	auto rl = codec8_16.in(cs, ce, cc, wb, wb + rbsz / ESize, wc);
	cc16.f_toupper(wb, wc);
	auto cpysz = (wc - wb)*ESize;
	if (rl >= std::codecvt_base::error) {
		auto asz = std::min<size_t>(ce - cc, (wb + rbsz / ESize - wc)*ESize);
		memcpy(wc, cc, asz);
		cc += asz;
		cpysz += asz;
	}
	return cpysz;
};
};

namespace tbgeneral{

	// KOI8 <> w1251
static const byte koi8enc[128] = {
		0x3F, 0x3F, 0x27, 0x3F, 0x22, 0x3A, 0x8A, 0xBC, 0x3F, 0x25, 0x3F, 0x3C, 0x3F, 0x3F, 0x3F, 0x3F,
		0x3F, 0x27, 0x27, 0x22, 0x22, 0x07, 0x2D, 0x2D, 0x3F, 0x54, 0x3F, 0x3E, 0x3F, 0x3F, 0x3F, 0x3F,
		0x9A, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x81, 0x15, 0xB3, 0xBF, 0x3F, 0x3C, 0x83, 0x2D, 0x52, 0x3F,
		0x9C, 0x2B, 0x3F, 0x3F, 0x3F, 0xDE, 0x14, 0x9E, 0xA3, 0x3F, 0x3F, 0x3E, 0x3F, 0x3F, 0x3F, 0x3F,
		0xE1, 0xE2, 0xF7, 0xE7, 0xE4, 0xE5, 0xF6, 0xFA, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, 0xF0,
		0xF2, 0xF3, 0xF4, 0xF5, 0xE6, 0xE8, 0xE3, 0xFE, 0xFB, 0xFD, 0xFF, 0xF9, 0xF8, 0xFC, 0xE0, 0xF1,
		0xC1, 0xC2, 0xD7, 0xC7, 0xC4, 0xC5, 0xD6, 0xDA, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0,
		0xD2, 0xD3, 0xD4, 0xD5, 0xC6, 0xC8, 0xC3, 0xDE, 0xDB, 0xDD, 0xDF, 0xD9, 0xD8, 0xDC, 0xC0, 0xD1
};

size_t cpen_defCP2koi8( char * dest , const char * src){
	size_t len= strlen(src);
	for (char* p=dest  , *e=(char*)src+len; *src && (src<e); ++p,++src) 
		if (src[0]<0) p[0]=(char)koi8enc[0x7F&src[0]];
	return len;
};




}; // namespace tbgeneral




namespace tbgeneral{

	// -------- LOCALE --------------
namespace unicodens{
		static const char32_t INVALID_UNICODE = 0x110000;
		char32_t utf8_to_utf32(const char*& u8str) {
			uint u = (byte)*u8str++;
			if(u < 0x80) return  u ;
			else if(u < 0xC0) return INVALID_UNICODE;
			else if(u < 0xE0) {
				char32_t res = (u & 0x1F) << 6; u = (byte)*u8str++;
				if(u < 0x80 || u >= 0xC0) return INVALID_UNICODE;
				return res | (u & 0x3F) ;
			} else if(u < 0xF0) {
				char32_t res = (u & 0x0F) << 12; u = (byte)*u8str++;
				if(u < 0x80 || u >= 0xC0) return INVALID_UNICODE;
				res |= (u & 0x3F) << 6; u = (byte)*u8str++;
				if(u < 0x80 || u >= 0xC0) return INVALID_UNICODE;
				return res | (u & 0x3F);
			} else if(u < 0xF8) {
				char32_t res = (u & 0x07) << 18; u = (byte)*u8str++;
				if(u < 0x80 || u >= 0xC0) return INVALID_UNICODE;
				res |= (u & 0x3F) << 12;	u = (byte)*u8str++;
				if(u < 0x80 || u >= 0xC0) return INVALID_UNICODE;
				res |= (u & 0x3F) << 6;		u = (byte)*u8str++;
				if(u < 0x80 || u >= 0xC0) return INVALID_UNICODE;
				return res | (u & 0x3F);
			} else return INVALID_UNICODE;
		}
}
bool utf8_Validate(const stringA & src , stringA * validpart){
	const char* ts= src.cbegin(); char32_t r=0;
	for(;ts < src.end() && r!= unicodens::INVALID_UNICODE; ) 
		r = unicodens::utf8_to_utf32(ts);
	if (validpart) *validpart = src;
	if (r!= unicodens::INVALID_UNICODE) return true;
	if (validpart) validpart->set_end(ts); 
	return false;
}
	


size_t wide_to_utf8_b(const wchar * w , size_t wsz , size_t * cntread_wc , char* buff , size_t csz ) {
	codec_utf8_utf16 cd;
	auto wc = w; auto cc = buff;
	cd.out(w, w + wsz, wc, buff, buff + csz, cc);
	if (cntread_wc) *cntread_wc = wc - w;
	return cc-buff;
};


char* wide_to_utf8(  char* buff , size_t buffsz , wchar w  ){
	auto cnt = wide_to_utf8_b( &w , 1 , 0, buff , buffsz-1 );
	buff[cnt]=0;
	return buff;
};


stringA wide_to_utf8( const wchar* s ){
	return wide_to_utf8( s, strlen_s(s) );
}
size_t wide_to_utf8(  stringA & res , wchar w   ){
	return wide_to_utf8( res , &w, 1); 
};

stringA wide_to_utf8( const wchar* s , size_t size ){
	stringA res; 
	wide_to_utf8_add( res , s , size);
	return res;
}

size_t wide_to_utf8( stringA & buff , const wchar *w , size_t cnt  ){
	buff.resize(0);
	wide_to_utf8_add( buff , w, cnt);
	return buff.size();
};

stringA wide_to_utf8_add( stringA & res  , const wchar *wstart , size_t cnt_w ) {
	enum { CharBuffLength= 4*1024};
	codec_utf8_utf16 cd;
	char buff[CharBuffLength];
	auto w=wstart, wc = w , we =w+cnt_w  ; auto cc = buff; auto sz_ch=std::size(buff);
	auto oldsize_c = res.size();
	res.reserve(oldsize_c+cnt_w);
	for (w=wc; w<we; w=wc ) {
		cd.out(w, we, wc, buff, buff+sz_ch  , cc);
		//TODO: стратегия резервирования! можно резервировать в соответствии с остатком (we-wc)
		res.append( buff , cc-buff );
	}
	return res.slice( oldsize_c  );
}


stringA wide_to_utf8_add(stringA & buff,  wchar w   ) {
	return wide_to_utf8_add( buff , &w , 1 );
}

//}; // namespace tbgeneral 



static int char_codepage_type=eUTF8; //''
int  get_char_codepage_type( ){ return char_codepage_type; }
int  set_char_codepage_type( int newt){
	//auto old= char_codepage_type;
	char_codepage_type= newt;
	return 0;
}

stringW utf8_to_wide( const stringA & s ) { 
	return utf8_to_wide(s.data(), s.size()); };

stringW utf8_to_wide( const char* s ){
	return utf8_to_wide(s,strlen(s));
};

stringW utf8_to_wide( const char* s , size_t size ){
	stringW wr; 
	utf8_to_wide_add( wr , s , size);
	return wr;
};

size_t utf8_to_wide(stringW& dest, const char* src , size_t size ) {
	dest.resize(0);
	utf8_to_wide_add( dest, src , size );
	return dest.size();
}

size_t utf8_to_wide(stringW& dest, const stringA& src) {
	return utf8_to_wide( dest , src.cbegin() , src.size() );
}

size_t utf8_to_wide_add(stringW& dest, const char* src , size_t size ) {
	enum { eLocalBuffCount= 1024 };
	//TODO: Minimize memory
	codec_utf8_utf16 cd;
	size_t cntadded;
	if (size>eLocalBuffCount) {
		auto oldsize= dest.size();
		dest.resize(oldsize+size);
		const char* cc; auto ws = dest.data()+oldsize, wc = ws;
		cd.in( src , src+size , cc, ws, ws + size , wc);
		cntadded = wc - ws;
		dest.resize(oldsize+cntadded);
	} else {
		wchar_t buff[eLocalBuffCount];
		auto cc=src; auto wc = buff; 
		cd.in( src , src+size , cc, buff, buff + eLocalBuffCount , wc);
		cntadded = wc - buff;
		dest.append( buff , cntadded );
	}
	return cntadded;
}






void append_convert(stringW & dest , const char * src , size_t size){
	//int utf8_to_wide_add(stringW& dest, const char* src , uint size )
	if (!size) return;
	if (char_codepage_type==eUTF8) {
		utf8_to_wide_add(dest, src , size );
	} else {
		auto oldsz = dest.size();
		dest.resize(oldsz+size);	
		deflocale_to_utf16( dest.data()+oldsz , size + 1, src, size );
	}
}

void append_convert(stringA & dest , const wchar_t * src , size_t srcsz){
	if (!srcsz) return;
	if (char_codepage_type==eUTF8) {
		wide_to_utf8_add(  dest , src , srcsz    );	
	} else {
		auto oldsz = dest.size();
		dest.resize(oldsz+srcsz);
		if (!srcsz) return;
		utf16_to_deflocale(dest.data()+oldsz, srcsz , src, srcsz);
	}
}

void convert(stringW & dst , const char * src , size_t size){
	dst.resize(0);
	append_convert( dst , src , size);
};

void convert(stringA & dst , const wchar_t * src , size_t size){
	dst.resize(0);
	append_convert( dst , src , size);
};



stringA encode_str_to_consoleCP( const char * s , size_t sz ){
	if (!sz) { sz= s ? strlen(s) : 0; }
	stringA res; res.resize( sz );
	encode_str_to_consoleCP( res.data(), res.size(), s );
	return res;
}
stringA encode_str_to_consoleCP( stringA  s ){
	return encode_str_to_consoleCP( s.data() , s.size() );	
}

void con_print( const char * s  ){
	if (get_char_codepage_type()==eUTF8) {
		std::cout << s;
	} else {	
		auto r= encode_str_to_consoleCP( s , 0 );
		std::cout << r;
	}
}
/*
void vcon_printf( const char * fmt , va_list argptr ){
	if (get_char_codepage_type()==eUTF8) {
		vprintf(fmt, argptr );
	} else {	
		auto s= vformat( fmt , argptr);
		s= encode_str_to_consoleCP( s  );
		printf("%s", s );
	}
}

void con_printf( const char * fmt , ... ){
	va_list argptr; va_start(argptr,fmt);
	vcon_printf( fmt , argptr);
}
*/

/*
void con_print( const wchar * s  ){
	if (get_char_codepage_type()==eUTF8) {
		con_print( wide_to_utf8( s ) );
	} else {
		wprintf(L"%s", s );
	}
}
void vcon_printf( const wchar * fmt , va_list argptr  ){
	if (get_char_codepage_type()==eUTF8) {
		auto r= vformat(fmt, argptr);
		con_print( wide_to_utf8( r ) );
	} else {
		wprintf( fmt , argptr );
	}
}

void con_printf( const wchar * fmt , ... ){
	va_list argptr; va_start(argptr,fmt);
	vcon_printf( fmt , argptr);
}
*/



//----------------- UPPER/LOWER CASE -----------------------

//auto locale_utf8 = std::locale("en_US.utf8");
//auto locale_utf8 = std::locale();
//auto use_facet_w_utf8 = &std::use_facet<std::ctype<wchar_t>>(locale_utf8);
//auto use_facet_c_utf8 = &std::use_facet<std::ctype<char>>(locale_utf8);


//TODO: CASE CHANGE
void f_strtoupper( char * l , size_t sz) { 	codec_case_utf8 cd; cd.f_toupper(l, l + sz);  }
void f_strtolower( char * l , size_t sz) { codec_case_utf8 cd; cd.f_tolower(l, l + sz); }
void f_strtoupper(wchar_t * l, size_t sz) { codec_case_utf16 cd; cd.f_toupper(l, l + sz); }
void f_strtolower(wchar_t * l, size_t sz) { codec_case_utf16 cd; cd.f_tolower(l, l + sz); }

//static int do_strnicmp_ll( const wchar_t * l , const wchar_t * r , size_t sz ){	
static int do_strnicmp_l(const wchar_t * l, const wchar_t * r, size_t szl, size_t szr) {
	codec_case_utf16 cd;
	return cd.icmp(l, r, szl, szr );
};
static int do_strnicmp_l(const char * l, const char * r, size_t szl , size_t szr ) {
	codec_case_utf8 cd;
	return cd.icmp(l, r, szl , szr );
};

template<typename charT> int t_stricmp_l(const charT * l , const charT * r , size_t szl, size_t szr){
	return do_strnicmp_l( l,  r, szl, szr); //TODO: error NEED  do_strnicmp_l( l,  r, szl , szr );
};

int f_stricmp(const wchar_t * l , const wchar_t * r , size_t lsz , size_t rsz) { return t_stricmp_l(l,r,lsz,rsz); };
int f_stricmp(const char * l , const char * r , size_t lsz , size_t rsz) { return t_stricmp_l(l,r,lsz,rsz); };






}; // namespace tbgeneral{

#include "t_string.h"

namespace tbgeneral {
//int utf8_to_wide(rc_string<wchar_t>& dest, const rc_string<char>& src);
//uint wide_to_utf8(const wchar* w, uint wsz, rc_string<char>& res);


size_t sarr_convert(wchar_t* dest, size_t destsize, const char* src, size_t srcsize , size_t &readsz) {
	if (char_codepage_type == eUTF8) {
		codec_utf8_utf16 cd;
		const char* ccurr=src; auto  wc = dest;
		cd.in(src, src+srcsize , ccurr , dest , dest+destsize , wc);
		readsz = ccurr - src;
		return wc - dest;
	} else {
		auto sz = std::min(destsize, srcsize);
		deflocale_to_utf16( dest , destsize , src, srcsize);
		readsz = sz;
		return sz;
	}
};
size_t sarr_convert(char* dest, size_t destsize, const wchar_t* src, size_t srcsize , size_t & readsz ) {
	if (char_codepage_type == eUTF8) 
		return wide_to_utf8_b(src, srcsize, &readsz, dest, destsize );
	else {
		auto sz = std::min(destsize, srcsize);
		auto res = utf16_to_deflocale(dest, destsize, src, srcsize);
		readsz = sz;
		return sz;
	}
};

/*
void convert(rc_string<char>& dst, const wchar_t* src, size_t srcsz) {
	if (char_codepage_type == eUTF8) {
		//dst= wide_to_utf8( src , srcsz );	
		wide_to_utf8(src, srcsz, dst);
	}
	else {
		dst.resize(srcsz * 2);
		if (!srcsz) return;
		utf16_to_deflocale(&dst[0], dst.size(), src, srcsz);
		//sprintf_s( &dst[0] , dst.size() , "%S" , src );
		dst.resize(srcsz);
	}
};
void convert(rc_string<wchar_t>& dst, const char* src, size_t srcsz) {
	if (char_codepage_type == eUTF8) {
		rc_string<char> ss;
		ss.assign_cnstval( src , srcsz);
		utf8_to_wide( dst , ss ); 
		//dst = utf8_to_wide(src, srcsz);
	}
	else {
		dst.resize(srcsz);	if (!srcsz) return;
		deflocale_to_utf16(&dst[0], dst.size() + 1, src, srcsz);
	}
};
*/
}; // namespace tbgeneral{




