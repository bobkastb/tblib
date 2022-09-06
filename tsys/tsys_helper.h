#pragma once

#include "gdata/t_string.h"

namespace tbgeneral {
	//using stringA= tbgeneral::stringA;  
	//using stringW = tbgeneral::stringW;
	//using param_stringA = tbgeneral::param_stringA;

	stringW buffWfromAstr(const param_stringA& p, void* b, size_t bsize); 
	template<size_t N> stringW buffWfromAstr(const param_stringA& p, wchar_t(&b)[N]) {
		return buffWfromAstr(p, b, N * sizeof(wchar_t));
	}
	template<size_t N> stringA buffAfromAstr(const param_stringA& p, char(&b)[N]) {
		stringA r;
		r.assign_storage( b , N*sizeof(char));
		r.assign(p.param);
		return r;
	}

}
