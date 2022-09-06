//#include "tbt_strings.h"
//#include "tb_parray.h"
#include <cctype>
#include <cwctype>
#include "tb_vector_cs.h"
#include "t_string.h"



namespace tbgeneral{

	void bibbf( const char * mess  , ...   ) { RAISE( mess ); }; 

stringA find_str(const stringA & s , const stringA & substr ) {
		auto st=s.data(), e=st+s.size() , sb=substr.data(); //, sbe=sb+substr.size();
		auto sbsz=substr.size();
		stringA res;
		for(;1;) {
			st=(const char*)memchr( st,*sb, e-st);
			if (!st) return res;
			if (0==memcmp(st,sb,sbsz)) {
				//res=s; res.s=st; 
				return s.slice(st, st+sbsz);
			}
			st++;
		}
		return res;
}

stringW find_str(const stringW& s, const stringW& substr) {
		auto st = s.data(), e = st + s.size() - substr.size() + 1, sb = substr.data(); //, sbe=sb+substr.size() ;
		auto sbsz = substr.size(), ssbsz = sbsz * sizeof(*sb);
		stringW res;
		if (sbsz == 0) return res;
		for (; st < e; st++) {
			if (*st != *sb) continue;
			if (sbsz == 1 || 0 == memcmp(st, sb, ssbsz)) {
				return s.slice(st);
			}
		}
		return res;
	};

template<class PString>	PString replace_str_t(const PString & _s , const PString & from , const PString & to ) {
		auto r= find_str(_s,from); if (0==r.size()) return _s;
		typename PString::string_type news; news.reserve(_s.size()); 
		auto s=_s;
		auto fr_sz= from.size();
		for(;r.size();){
			news.append( s.cbegin() ,r.cbegin());
			news.append(to);
			s.set_begin(r.cbegin() + fr_sz);
			r=find_str(s,from);
		}
		news.append(s);
		return news;
}



stringA replace_str(const stringA& _s, const stringA& from, const stringA& to) {
	return replace_str_t(_s, from, to);
}
stringW replace_str(const stringW & _s , const stringW & from , const stringW & to ){
	return replace_str_t(_s,from,to);
};

darray<stringA> split_str(const stringA & s , const stringA & substr ) {
	auto sc = s, sn=s; darray<stringA> res;
	for (;true;) {
		sn = find_str(sc,substr);
		if (sn.size()) {
			res.push_back( s.slice(sc.cbegin(), sn.cbegin()));
			sc.set_begin( sn.cbegin() + substr.size() );
		} else {
			res.push_back( sc );
			break;
		}
	}
	return res;
}

int split_str2( const stringA & d, char splitset , stringA & l , stringA & r ){
	stringA res[2];
	auto s=d.data(),e=s+d.size(),cs=s;
	int cnt=0;
	for (;s<e;s++){	
		if (*s==splitset) { 
			if (cs<s) {res[cnt] = d.slice(cs,s); }
			cs=s+1; cnt++; s=e;
			break;
		}
	}
	if (cs<s) {res[cnt] = d.slice(cs, s); }
	l=res[0]; r=res[1];
	return cnt;
};



darray<stringA> split_str( const stringA & d, char splitset ){
	darray<stringA> res;
	auto s=d.data(),e=s+d.size(),cs=s;
	for (;s<e;s++){	
		if (*s==splitset) { 
			if (cs < s) { res.push_back(d.slice(cs, s)); }
			cs=s+1;
		}
	}
	if (cs<s) {res.push_back(d.slice(cs, s));}
	return res;
};


bool isspaceChar( char c ) { return  c>0 && 0!=std::isspace(c); }
bool isspaceChar( wchar_t c ) { return 0!=std::iswspace(c); }

template <class TChar> rc_string<TChar> lstring_trim_( const rc_string<TChar> & ss ){
	const TChar * s=ss.data(), *e= s+ss.size();
	for(; (s<e) && (isspaceChar(*s)) ; s++);
	for(; (s<e) && (isspaceChar(e[-1])) ; e--);
	return rc_string<TChar>( s, e-s );

};

/*
template <class TChar> partstring<TChar> partstring_trim_(const partstring<TChar>& ss) {
	//TODO:
	const TChar* s = ss.data(), * e = s + ss.size();
	for (; (s < e) && (isspaceChar(*s)); s++);
	for (; (s < e) && (isspaceChar(e[-1])); e--);
	return partstring<TChar>(s, e - s);
};
*/


stringA lstring_trim( const wchar_t * s){	return lstring_trim_( stringA(s) ); }
stringA lstring_trim( const char * s){	return lstring_trim_( stringA(s) ); }
//partstringA lstring_trim(const partstringA& s) { return partstring_trim_(s); }
stringA lstring_trim( const stringA & s){	return lstring_trim_(s); }
stringW lstring_trim( const stringW & s){	return lstring_trim_(s); }





//TODO: linux!
// TODO locale & utf8
//----------------- UPPER/LOWER CASE -----------------------

/*
int stricmp( const char * s , const char * d ){
	if (!s) s="";
	if (!d) d="";
#ifdef _WIN32
	return _stricmp(s,d);
#else 
	return strcasecmp(s,d);
#endif
};
*/

//-----------------
template<typename charT> int do_strcmp( const charT * l , const charT * r , size_t szl, size_t szr ){
	size_t sz=std::min( szl ,szr );
	for (;sz;sz--,l++,r++) { if (*l<*r) return -1; if (*l>*r) return 1; }
	return szl<szr ? -1 : szl>szr ? 1 : 0;
};
int strcmp( const char * l , const char * r , size_t lsz , size_t rsz ){return do_strcmp(l,r,lsz,rsz);};
int strcmp( const wchar_t * l , const wchar_t * r , size_t lsz , size_t rsz ){return do_strcmp(l,r,lsz,rsz);};

void memcopy_nwarn( void* dist , const void* src , size_t size){
	memcpy( dist , src , size );
};
void memmove_nwarn( void* dist , const void* src , size_t size){
	memmove( dist , src , size );
};

void memreset( void* data , size_t size){
	memset( data , 0 , size );
};


// ------------------- 

stringA replacechars( const stringA & src , const stringA & set , const stringA & repl ){
	stringA result; result.reserve(src.size());
	cBitSetC<256> bits(set);
	const char* s=src.data(),*e=s+src.size(),*sf=s;
	for (; s<e; s++){
		if (bits.test(byte(*s))) { 
			result.append(sf,s-sf); result << repl; sf=s+1; 
		} 
	}
	result.append(sf,s-sf);
	return result;
}

} // namespace tbgeneral{

