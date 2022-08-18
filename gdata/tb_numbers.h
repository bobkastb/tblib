#pragma once 

#include "tsys/tb_sysdecls.h"
#include "tb_basetypes.h"
#include "tb_basedtools.h"
#include "t_string.h"
#include <cfloat>


namespace tbgeneral{

template< class C > bool cNumericType_IsSigned() { return C(-1)<0; }; 
template< class C > C cNumericType_MaxValue() {  return C(-1)>0 ? C(-1) :  C(-1) ^ C( 0 ^ (C(1)<<( sizeof(C)*8-1 ) )) ; }; 
template< class C > C cNumericType_MinValue() {  return C(-1)>0 ? 0 :  C( 0 ^ (C(1)<<( sizeof(C)*8-1 ) )) ; }; 

template< class C > struct cNumericTypeMetaInfo { enum{ Signed=C(-1)<0 ? 1 : 0  , Float=0,
		Max = C(-1)>0 ? C(-1) :  C(-1) ^ C( 0 ^ (C(1)<<( sizeof(C)*8-1 ) )) , 
		Min = C(-1)>0 ? 0 :  C( 0 ^ (C(1)<<( sizeof(C)*8-1 ) ))
		};   
		static C getMax() { return Max; }
		static C getMin() { return Min; }
}; 
#define DEf_cNumericTypeMetaInfo_FLT( CType , _Max  ) 	template<> struct cNumericTypeMetaInfo<CType>{ enum{ Signed=1 ,Float=1 }; static CType getMax() { return _Max; }; static CType getMin() { return -_Max; }; } 
#define DEf_cNumericTypeMetaInfo_I64( CType ) 	template<> struct cNumericTypeMetaInfo<CType>{ enum{ Signed=CType(-1)<0 ? 1 : 0 ,Float=0 }; \
	static CType getMax() { return cNumericType_MaxValue<CType>(); }; \
	static CType getMin() { return cNumericType_MinValue<CType>(); }; \
	} 
DEf_cNumericTypeMetaInfo_FLT( double , DBL_MAX );
DEf_cNumericTypeMetaInfo_FLT( float , FLT_MAX );
DEf_cNumericTypeMetaInfo_I64( int64 );
DEf_cNumericTypeMetaInfo_I64( uint64 );
const char* cNumericType_gettypenameb( size_t sz , int Signed , int Float );	
template<class Nt> const char * cNumericType_gettypename(){ typedef cNumericTypeMetaInfo<Nt> minft;
	 return cNumericType_gettypenameb( sizeof(Nt) , minft::Signed , minft::Float );};	

struct str2number_c {
	enum { Ok = 0 , Overflow = 0x10000, Underflow  };
};

template< class TRi , class TSi > int comparenum_CHKOVER( TRi ri , TSi si ){
	warning_PUSH()
	warning_MSVC(disable,4018)
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif

	if ((ri>=0)&&(si<0)) return 1;
	if ((ri<0)&&(si>=0)) return -1;
    return ri<si ? -1 : ri==si ? 0 : 1;
	warning_POP()
};
template< class TRi , class TSi > int convertnum_CHKOVER( TRi & ri , const TSi & si ){
	warning_PUSH()
	warning_MSVC(disable,4244)
	if ( comparenum_CHKOVER( si, cNumericTypeMetaInfo<TRi>::getMax())>0 ) return str2number_c::Overflow;
	if ( comparenum_CHKOVER( si, cNumericTypeMetaInfo<TRi>::getMin())<0 ) return str2number_c::Underflow;
//	if ( comparenum_CHKOVER( si, cNumericType_MaxValue<TRi>())>0 ) return 1;
//	if ( comparenum_CHKOVER( si, cNumericType_MinValue<TRi>())<0 ) return -1;
	ri = si;
	warning_POP()
	return 0;
}
struct str2number_result{
	enum { isErr, isI64=100 , isUI64 , isDouble };
	int8 restp;
	union{
		uint64 i64;
		double fdouble;
	};
	int aresult( int64 r) { restp=isI64; i64=r; return 0;};
	int aresult( uint64 r) { restp=isUI64; i64=r; return 0;};
	int aresult( double r) { restp=isDouble; fdouble=r; return 0;};
	template <typename Numt> int convert( Numt * res) const {
		switch( restp ) {
		  case isI64 :  return convertnum_CHKOVER(*res , i64 ); 
		  case isUI64 :  return convertnum_CHKOVER(*res , uint64(i64) ); 
		  case isDouble :  return convertnum_CHKOVER(*res , fdouble ); 
		  default: return 1;	
		};
	}
};
int str2number_x(const char * s, size_t sz , str2number_result & res );
int str2number_x(const wchar_t * s, size_t sz , str2number_result & res );

template <typename C, typename Numt> int str2number(const C* s, size_t szC , Numt * res ){ 
		str2number_result sr;
		int r=str2number_x( s, szC , sr ); if (r) return r;
		return sr.convert( res);
	return 1;
 };
template <typename C, typename Numt> int str2number(const C* s,  Numt * res ){ return str2number( s, tbgeneral::strlen_s(s) , res ); }
template <typename Numt> int str2number(const stringA & s, Numt* res) { return str2number(s.cbegin(), s.size() , res); }


stringA & operator << (stringA & s , int x);

void convert(rc_string<char>& res, int64 d);
void convert(rc_string<wchar_t>& res, int64 d);
void convert(rc_string<char>& res, uint64 d);
void convert(rc_string<wchar_t>& res, uint64 d);
void convert(rc_string<char>& res, double d);
void convert(rc_string<wchar_t>& res, double d);

stringA itoa_t( int64 d );
stringA uitoa_t( uint64 d );
stringA ftoa_t( double d );

stringA arr2hex(const byte* dptr, size_t dsize , bool lcase=false);
inline stringA arr2hex(const darray<byte>& d, bool lcase = false) { return arr2hex(d.data(), d.size(), lcase); }


int atoi_t( const char * s , const char * se, int64 & res );
template <class I> int atoi_t( const param_stringA & str , I & res ){ 
	int64 v;
	auto err= atoi_t( str.param.cbegin(), str.param.cend() ,v );
	res= I(v);
	return err;
}; 


}; //namespace tbgeneral{