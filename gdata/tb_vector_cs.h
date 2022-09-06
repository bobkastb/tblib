#pragma once

#include <tb_basetypes.h>
#include "t_string.h"

namespace tbgeneral {

template <class T,int cnt> struct cVectorC{
	enum {MaxSize=cnt};
	typedef T element_t;
	typedef T value_type;
	int sz;
	T data[cnt];
	cVectorC() { ZEROMEM(*this); }
	uint size() const { return sz; }
	void clear() { resize(0); }
	void resize( int nsz ) { checkrange(nsz); sz=nsz; }; 
	const T& operator [] (int b) const  { checkrange(b); return data[b]; }
	T& operator [] (int b) { checkrange(b); return data[b]; }
	bool checkrange(int b) const { if (b<0 || b>=cnt) throw 155; return true; };
	int push_back(const T& v) {checkrange(sz);data[sz]=v;sz++; return sz-1;};
	template<class TStr> bool strassign( const TStr & d){
		uint csz= std::min( uint(sz) , uint(d.size() ));
		if (csz==sz) { csz--; }
		memcpy( data , d.data() , csz );
		data[csz]=0;
		return csz>=uint(d.size()) ; //no truncate
		}
	};

// tb_vector_cs.h , for cBitSetC
void init_bitset_data( void * data , uint datasize, const stringA & bitnumbers_list );

template <int bcnt=256> struct cBitSetC{
	enum {bsz=bcnt>>5};
	typedef cBitSetC<bcnt> thistype;
	uint32 data[bsz];
	cBitSetC() { ZEROMEM(*this); }
	cBitSetC(const stringA & bitnumbers_list) { init_bitset_data(data,sizeof(data),bitnumbers_list); }
	//TODO: cBitSetC(const char* bitnumbers_list,size_t sz) { init_bitset_data(data, sizeof(data), bitnumbers_list); }
	bool checkrange(int b) const { if (b<0 || b>=bcnt) throw 155; return true; };
	void set( int bitnum ) { checkrange(bitnum); data[bitnum>>5] |= 1 << (bitnum & 0x1f); }  
	void reset( int bitnum ) { checkrange(bitnum); data[bitnum>>5] &= ~(1 << (bitnum & 0x1f)); }  
	bool test(int bitnum) const { checkrange(bitnum); return 0!= ( data[bitnum>>5] & ( 1 << (bitnum & 0x1f))); }
	void clear() { ZEROMEM( data ); };
	bool empty() const { const uint32 * d=data ,*e=d+bsz; for (;d<e;d++) if (*d) return false; return true; } ;
	thistype operator & ( const thistype & s2 ) const 
		{ 	thistype r;	for (int i=0;i<bsz;i++) r.data[i]=data[i] & s2.data[i]; return r;	}
	thistype operator | ( const thistype & s2 ) const 
		{ 	thistype r;	for (int i=0;i<bsz;i++) r.data[i]=data[i] | s2.data[i]; return r;	}
	thistype operator - ( const thistype & s2 ) const 
		{ 	thistype r;	for (int i=0;i<bsz;i++) r.data[i]=data[i] & ~s2.data[i]; return r;	}
	bool operator == ( const thistype & s2 ) const 
		{ 		for (int i=0;i<bsz;i++) if (s2.data[i]!=data[i]) return false; return true;	}
	template<class Vec> int FromVector( Vec vec ) { clear(); for (uint i=0;i<vec.size();i++) set(vec[i]); return vec.size(); };
	};

template<class Vec, class VT> int vectorFindValue( const Vec & vec , const VT & val ){
	for (uint i=0;i<vec.size();i++) 
		if (vec[i]==val) return i; 
	return -1;
}


}; // namespace