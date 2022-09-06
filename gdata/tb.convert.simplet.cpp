#include <stdio.h>
#include <math.h>
#include "tb_numbers.h"
#include "gdata/t_string.h"

// for tests



//************************
namespace tbgeneral{ 

template <typename C> int str2number_xx(const C* s, size_t sz , str2number_result & res ){
	enum { numOVERFLOW= str2number_c::Overflow };
	//int base=10; 
	if ((s==0)||(sz==0)) return -1; 
	const C* cur=s, * en=s+sz;
	if ((sz>2) && (cur[0]=='0')&&((cur[1]=='x')||(cur[1]=='X'))) { //base=16; 
		cur+=2;  
		uint64 resv=0;
		for( ; cur<en ; cur++){ C cc=cur[0]; int v;
			v= (cc>='a' && cc<='f') ? cc-'a'+10 : (cc>='A' && cc<='F') ?  cc-'A'+10 : (cc>='0' && cc<='9') ? cc-'0' : -1;
			if (v<0) return (int)(cur-s+1);
			resv = (resv << 4) + v;
		 };
		res.aresult( uint64(resv)) ;	
		return 0;
	};
	uint64 resui=0; double resf, rfrac; int sgn=1;	int32 degree;
	if ((*cur=='-')||(*cur=='+')) {sgn=*cur=='-' ? -1 : 1; cur++; }
	for( ; cur<en ; cur++){ C cc=cur[0]; 
		if (cc>='0' && cc<='9') resui=resui*10 + (cc-'0'); 
		else if (cc=='.') break;
		else if (cc=='e' || cc=='E') break;
		else return (int)(cur-s+1);
	};
	if (cur==en) { if (sgn==-1 && 0!=(resui >> 63)) return numOVERFLOW;  
		if (sgn==-1) res.aresult( -int64(resui)); else res.aresult( uint64(resui)); 
		return 0; 
	}
	resf= double(resui)*sgn;
	rfrac=0;degree=0; if (cur[0]=='.' ) for( cur++ ; cur<en ; cur++){ C cc=cur[0]; 	
		if (cc>='0' && cc<='9') { rfrac=rfrac*10 + (cc-'0'); degree++; }
		else if (cc=='e' || cc=='E') break;
		else return (int)(cur-s+1); 	
	};
	resf = resf +  double(sgn)*rfrac / pow( double(10) , double(degree) );
	if (cur<en) { degree=0; cur++; sgn=1; if ((*cur=='-')||(*cur=='+')) { cur++; sgn= cur[0] == '-' ? -1 : 1; }
	 for(  ; cur<en ; cur++){ C cc=cur[0]; 	
			if (cc>='0' && cc<='9') { degree=degree*10 + (cc-'0'); }
			else return (int)(cur-s+1);
		};
		resf = resf * pow(double(10),double(sgn*degree));
	};
	res.aresult( resf );
	return 0;
};
int str2number_x(const char * s, size_t sz , str2number_result & res ) { return str2number_xx(s,sz,res);};
int str2number_x(const wchar_t * s, size_t sz , str2number_result & res ){ return str2number_xx(s,sz,res); };

const char* cNumericType_gettypenameb( size_t sz , int Signed , int Float ){
	switch (sz) {
		case 1: return Signed ? "int8" : "uint8";
		case 2: return Signed ? "int16" : "uint16";
		case 4: return Float? "float" : Signed ? "int32" : "uint32";
		case 8: return Float? "double" : Signed ? "int64" : "uint64";
	};
	return "ErrorNumericType";
};
	
}; // namespace tbgeneral{

//===============================================

namespace tbgeneral{


template <class Strt > void convert__s2i(Strt & res, void* d , const char * frmt ){ 
	char buff[256];
	snprintf( buff, sizeof(buff) , frmt , *(int64*)d );
	res = buff;
};
template <class Strt > void convert__s2f(Strt & res, double d ){ 
	char buff[256];
	snprintf( buff, sizeof(buff) , "%f" , d );
	res = buff;
};

void convert(stringA & res, int64 d ){ convert__s2i( res , &d , "%I64d"); };
void convert(stringW & res, int64 d ){ convert__s2i( res , &d , "%I64d"); };
void convert(stringA & res, uint64 d ){ convert__s2i( res , &d , "%I64u"); };
void convert(stringW & res, uint64 d ){ convert__s2i( res , &d , "%I64u"); };
void convert(stringA & res, double d ){ convert__s2f( res , d ); };
void convert(stringW & res, double d ){ convert__s2f( res , d ); };


stringA itoa_t( int64 d ) { stringA res; convert( res , d); return res; };
stringA uitoa_t( uint64 d ){ stringA res; convert( res , d); return res; };
stringA ftoa_t( double d ){ stringA res; convert( res , d); return res; };


stringA arr2hex( const byte* dptr , size_t dsize , bool lcase) {

	const char* tabc_HC = "0123456789ABCDEF";
	const char* tabc_LC = "0123456789abcdef";
	auto tabc = lcase ? tabc_LC : tabc_HC;
	stringA res; res.resize(dsize*2);
	for (uint k = 0; k < res.size(); k++) {
		byte b = dptr[k >> 1];
		res[k] = tabc[(k & 1) == 0 ? b >> 4 : b & 0xF];
	};
	return res;
}


}; // namespace tbgeneral{

namespace tbgeneral{
int atoi_t( const char * s , const char * se, int64 & res ){ 
	res=0; int64 rr=0,nx; int sign=1;
	if ( s>=se ) return 1;
	for (; s<se; s++) { auto c=*s;	if (!(c==' '||c=='	')) break; }
	for (; s<se; se--) { auto c=se[-1];	if (!(c==' '||c=='	')) break; }
	if (*s=='-') { s++; sign=-1;}
	if  (s[0]=='0' && s[1]=='x') { s+=2;
		for (;s<se;s++,rr=nx) { char c=char(*s); //byte ov;
			if (c>='0' && c<='9') c-='0';
			else if (c>='A' && c<='F') c-='A' - 10;
			else if (c>='a' && c<='f') c-='a' - 10;
			else return 1;
		  nx = rr*16 + c;	if (nx<rr) return 2;
		}
	} else for (;s<se;s++,rr=nx) { char c=char(*s);
		if (c<'0' || c>'9') return 1;
		nx = rr*10 + (c-'0');	if (nx<rr) return 2; // overflow
	};
	res=rr*sign;	
	return 0;
}

stringA & operator << (stringA & s , int x) { char buff[64]; sprintf(buff,"%d",x); s.append( buff ); return s;}
}; // namespace tbgeneral
