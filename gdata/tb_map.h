#pragma once
#include <map>
//#include "tb_parray.h"
#include "gdata/t_string.h"

namespace tbgeneral{

template <class TMap,class TKey> bool exists( const TMap & m,const TKey& k){
	return m.find(k) != m.end();
}

template <class TKey,class TValue> TValue get_ness( const std::map<TKey,TValue> & m, const TKey& k){
	auto f=m.find(k); 
	if (f == m.end()) { return TValue(); }
	return f->second;
}

template <class TMap, class TKey,class TValue> TValue get_def( const TMap & m, const TKey& k, const TValue& def){
	auto f=m.find(k); 
	if (f == m.end()) { return def; }
	return f->second;
}

template <class TKey,class TValue> bool get_val( const std::map<TKey,TValue> & m, const TKey& k, TValue & res ){
	auto f=m.find(k); 
	if (f == m.end()) { return false; }
	res= f->second;
	return true;
}

template <class TStr> struct less_str_icase {
	bool operator()( const TStr& lhs, const TStr& rhs ) const {
		return 0>stricmp( lhs , rhs ) ; 
	};
};

template<class E> using smap = std::map<stringA,E,tbgeneral::less_str_icase<stringA>>;
template<class E> using spmap = std::map<stringA,E,tbgeneral::less_str_icase<stringA>>;


}; // namespace tbgeneral
