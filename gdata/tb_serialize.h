#pragma once


#include <map>
#include "t_string.h"
#include "tb_map.h"

// Пример и тесты : test/test_tblib/test_serialize.cpp

namespace tbgeneral{

	struct t_serializer {
		enum { eSerializer_Base= 0x1100,
			eSerializer_ReadInvalidArray = eSerializer_Base + 1,
			eSerializer_ReadEOF = eSerializer_Base + 2
		};
		typedef size_t t_label;
		virtual size_t write( const void* data , size_t sz  )=0;
		virtual size_t read( void* data , size_t sz  )=0;
		virtual size_t getpos()=0;
		virtual size_t size()=0;
		virtual bool seek(size_t newpos)=0;

		virtual bool checkread( size_t readsz , size_t * maxsz );
		virtual t_label make_start_crc();
		virtual bool fix_end_crc( t_label l );
		virtual bool check_crc();
		virtual bool calc_crc_cmp(t_label start , t_label end , byte *buff , size_t buffsz );

	};

	struct t_mem_serializer : t_serializer {
		darray<byte> buff;
		size_t r_position;
		t_mem_serializer(){ r_position=0; };
		t_mem_serializer(size_t initsize){ r_position=0; buff.reserve(initsize); };
		t_mem_serializer(const darray<byte> & _buff) { r_position = 0; buff= _buff; };
		size_t size()override  { return buff.size(); };
		bool seek( size_t pos ) override { if (pos>buff.size()) return false;  r_position=pos; return true; }

		size_t write( const void* data , size_t sz  ) override;
		size_t read( void* data , size_t sz  ) override;
		size_t getpos() override { return r_position;};

	};

inline t_serializer & operator << (t_serializer & st , int v) { st.write( &v , sizeof(v)); return st; }
inline t_serializer & operator >> (t_serializer & st , int & v) { st.read( &v , sizeof(v)); return st; }
inline t_serializer & operator << (t_serializer & st , uint v) { st.write( &v , sizeof(v)); return st;}
inline t_serializer & operator >> (t_serializer & st , uint & v) { st.read( &v , sizeof(v));return st; }
inline t_serializer & operator << (t_serializer & st , float v) { st.write( &v , sizeof(v)); return st;}
inline t_serializer & operator >> (t_serializer & st , float & v) { st.read( &v , sizeof(v)); return st;}
inline t_serializer & operator << (t_serializer & st , double v) { st.write( &v , sizeof(v)); return st;}
inline t_serializer & operator >> (t_serializer & st , double & v) { st.read( &v , sizeof(v)); return st;}
inline t_serializer& operator << (t_serializer& st, char v) { st.write(&v, sizeof(v)); return st; }
inline t_serializer& operator >> (t_serializer& st, char& v) { st.read(&v, sizeof(v)); return st; }
inline t_serializer & operator << (t_serializer & st , wchar v) { st.write( &v , sizeof(v)); return st;}
inline t_serializer & operator >> (t_serializer & st , wchar & v) { st.read( &v , sizeof(v)); return st;}
inline t_serializer & operator << (t_serializer & st , bool v) { st.write( &v , sizeof(v)); return st;}
inline t_serializer & operator >> (t_serializer & st , bool & v) { st.read( &v , sizeof(v)); return st;}
inline t_serializer& operator << (t_serializer& st, int64_t v) { st.write(&v, sizeof(v)); return st; }
inline t_serializer& operator >> (t_serializer& st, int64_t& v) { st.read(&v, sizeof(v)); return st; }
inline t_serializer& operator << (t_serializer& st, uint64_t v) { st.write(&v, sizeof(v)); return st; }
inline t_serializer& operator >> (t_serializer& st, uint64_t& v) { st.read(&v, sizeof(v)); return st; }
inline t_serializer& operator << (t_serializer& st, int16_t v) { st.write(&v, sizeof(v)); return st; }
inline t_serializer& operator >> (t_serializer& st, int16_t& v) { st.read(&v, sizeof(v)); return st; }
inline t_serializer& operator << (t_serializer& st, uint16_t v) { st.write(&v, sizeof(v)); return st; }
inline t_serializer& operator >> (t_serializer& st, uint16_t& v) { st.read(&v, sizeof(v)); return st; }
//inline t_serializer& operator << (t_serializer& st, int8_t v) { st.write(&v, sizeof(v)); return st; }
//inline t_serializer& operator >> (t_serializer& st, int8_t& v) { st.read(&v, sizeof(v)); return st; }
inline t_serializer& operator << (t_serializer& st, uint8_t v) { st.write(&v, sizeof(v)); return st; }
inline t_serializer& operator >> (t_serializer& st, uint8_t& v) { st.read(&v, sizeof(v)); return st; }

template <class TArray>  t_serializer & push_array(t_serializer & st , const TArray & s) {	 
		using value_type = typename TArray::value_type;
		st<<(int)s.size();
		if (MI_ISBINARY(value_type))	st.write( s.data() , s.size()*sizeof(value_type) );	
		else FOREACH(p,s) st<<*p;
		return st;  }
template <class TArray>  int pop_check_array(t_serializer & st , TArray & s, int maxsize ) {	 
		using value_type = typename TArray::value_type;
		int sz;  st>>sz; 
		if (maxsize>=0 && sz>maxsize) return 1;
		if (MI_ISBINARY(value_type)) { size_t binsz=sz*sizeof(value_type);
			if (!st.checkread( binsz , 0 )) return 2;
			s.resize(sz);
			st.read( s.data() , binsz );
		} else { 
			if (!st.checkread( sz , 0 )) return 2;
			s.resize(sz);
			FOREACH(p,s) st>>*p;
		}
		return 0;  
	}
template <class TArray>  t_serializer & pop_array(t_serializer & st , TArray & s) {	 
		auto res= pop_check_array<TArray>(st, s , -1);
		if (res!=0) RAISE_I(t_serializer::eSerializer_ReadInvalidArray); 
		return st;
	}

template <class TChar>  t_serializer & operator << (t_serializer & st , const rc_string<TChar> & s) {	 
		return push_array( st , s ); }
template <class TChar>  t_serializer & operator >> (t_serializer & st , rc_string<TChar> & s) {
		return pop_array( st , s ); }
//template <class TValue>  t_serializer & operator << (t_serializer & st ,const  parray<TValue> & s) { return push_array( st , s ); }
template <class TValue>  t_serializer & operator << (t_serializer & st ,const  darray<TValue> & s) {	 
		return push_array( st , s ); }
template <class TValue>  t_serializer & operator >> (t_serializer & st , darray<TValue> & s) {	 
		return pop_array( st , s ); }
template <class TKey,class TValue>  t_serializer & operator << (t_serializer & st ,const  std::map<TKey,TValue> & m) {	 
		st<<(int)m.size();
		FOREACH( p , m) st<<p->first<<p->second;
		return st;  }
template <class TKey,class TValue>  t_serializer & operator >> (t_serializer & st ,std::map<TKey,TValue> & m) {	 
		int sz; st>>sz;
		for(;sz>0;sz--) { TKey k;TValue v; st>>k<<v; m[k]=v; }
		return st;  }

}; // tbgeneral{
