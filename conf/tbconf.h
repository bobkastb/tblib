#pragma once
#include <stdint.h>
//#include <vector>
#include "gdata/t_string.h"
#include "gdata/t_sstruct.h"
#include "gdata/tb_env.h"
#include "gdata/tb_numbers.h"
namespace tbgeneral {	//namespace tbconf{





class tbconfig;
/*
template <class C> class shareptr{
	public:
	C * data;
	shareptr() { data=0; }
	shareptr & makenew() { data =new C(); return *this; }
	static shareptr donew() { return shareptr().makenew();}
	shareptr(const C* _d) { data=(C*)_d; }
	shareptr & operator = (const C* _d) { data=(C*)_d; }
};
*/

/*
namespace cTypeControl{
	inline bool typeisint( uint64 v ) { return true;};
	template<class A> bool typeisint( A v ) { return false;};
};*/

// TODO: нужен Rename
using listptr = rc_base_struct<tbconfig> ;

class tbconfig{
	public:
	using keytype = stringA;
	using strtype = stringA;
	class relem {
		public: // protected
		uint tag;
		keytype _key;
		strtype _value;
		listptr _sublist;
		relem() { tag=0; }
	};
	uintptr_t usertag;
	private:
	uint itag;
	darray<relem> list;
	friend struct r_linkctx;
	public:
	size_t size() { return list.size();}
	const relem & operator [] (size_t i) { return list[i];}
	const relem & get(size_t i) const { return list[i];}
	relem & get(size_t i) { return list[i];}
	void push_back(const relem & ne) { list.push_back(ne); };
	const relem & eget(const keytype & key ); 
	const relem & eget(const keytype & key ) const; 
	const relem & get(const keytype & key ) const; 
	relem elem(const keytype & key ) const { auto e=&eget(key); return e ? *e : relem(); }; 
	//relem & get(const keytype & key ); 
	tbconfig() {usertag=0;itag=0;};

	listptr sublist(const keytype & key ) const { return elem(key)._sublist; }; 
	listptr sublist(size_t key ) const { return get(key)._sublist; }; 
	private: 
	const relem & eget_(const keytype & key ) const; 


	public:
	size_t getsizeofval( char* nm  ); 
	int	 getvalue_int(  char* nm   );
	bool getvalue( const char* nm , const char* & value);
	bool getvalue( const char* nm , strtype & value );
	bool getvalue( const char* nm  , int & value );
	bool getvalue( const char* nm  , uint & value );
	bool getvalue( const char* nm  , uint64 & value );
	bool getvalue( const char* nm  , listptr & value );
	bool getvalue( const char* nm  , bool & value );
	bool getvalue( const char* nm  , uint16 & value ) { uint vv; bool r=getvalue(nm,vv); if (r) value=vv; return r; };
	template <typename Et> bool getvalue_ifex(const char* nm  , Et & value) {
		Et vv; bool res= getvalue(nm , vv); if (res) value=vv; return res;
	};
	
	//-------------- new interface ---------------
	template <class IType > bool decodeIval( const relem & e , IType & rv ) const {
			int er= atoi_t( e._value , rv );
			if (er) RAISE( "bad int in params");
			return er==0;
	;}
	bool decodeval( const relem & e , int & rv ) const { return decodeIval(e,rv); };
	bool decodeval( const relem & e , uint & rv )const { return decodeIval(e,rv); };
	bool decodeval( const relem & e , uint16 & rv )const { return decodeIval(e,rv); };
	bool decodeval( const relem & e , int16 & rv )const { return decodeIval(e,rv); };
	bool decodeval( const relem & e , uint64 & rv ) const{ return decodeIval(e,rv); };
	bool decodeval( const relem & e , int64 & rv ) const{ return decodeIval(e,rv); };
	bool decodeval( const relem & e , bool & rv ) const{  int v ; bool r=decodeIval(e,v); rv=v!=0; return r; };
	bool decodeval( const relem & e , strtype & rv ) const{ rv = e._value;  return true; };
	bool decodeval( const relem & e , const char* & rv ) const{ rv = e._value.c_str();  return true; };
	bool decodeval( const relem & e , listptr & rv ) const{ rv = e._sublist ;  return true; };
	template <class Vt> bool getval(size_t i, Vt& v)const			{ const relem & e=get(i);  return (&e ? decodeval(e,v) : false);  };
	template <class Vt> bool getval(const char* k, Vt& v)const		{ const relem & e=get(k);  return (&e ? decodeval(e,v) : false);  };
	template <class Vt> bool getval(const keytype & k, Vt& v)const	{ const relem & e=get(k);  return (&e ? decodeval(e,v) : false);  };

	static listptr parse_tbconf( const stringA & dataOrfile ,  LocalEnv * env=0 );
}; // class tbconfig{


	//TODO: Эти 4 функции надо прятать
	listptr load_tbconfig( const stringA & fn );
	listptr load_tbconfig_fromtxt( const stringA & text ,const stringA & file=0);
	listptr load_tbconfigINIF( const stringA & fn );
	listptr load_tbconfigINIF_fromtxt( const stringA & text ,const stringA & file=0);



//}}; // namespace tbgeneral::tbconf
} // namespace tbgeneral
