#pragma once

#include <stddef.h>

#ifndef _TB_ALGO_H_
#define _TB_ALGO_H_

#define FOREACH( It , St) for(auto It=St.begin();It!=St.end();It++)

namespace tbgeneral{

//template <class T> T* getdata(const std::vector<T> & v){	return v.size() ? &v[0] : 0; }
//inline const char* getdata(const std::string & v){ 	return v.size() ? &v[0] : 0; }


template<class TData> void swap_lr(TData & l , TData & r) { auto t=l; l=r; r=t; }

template <typename type> type min(const type& a, const type& b) {
	return a<b ? a : b;
}

template <typename type> type max(const type& a, const type& b) {
	return a<b ? b : a;
}

template <typename type> int compare(const type& l, const type& r) {
	return l < r ? -1 : l>r ? 1 : 0;
}

// intersection_mem result 1)  lp in rp , 2) rp in lp , 3) lp > rp , 4) lp < rp
int intersection_mem(const void* lp, size_t lsize, const void* rp, size_t rsize);
template <class TElem> int intersection_tmem(TElem* lp, size_t l_cnt, const TElem* rp, size_t r_cnt) {
	return intersection_mem(lp, l_cnt * sizeof(TElem), rp, r_cnt * sizeof(TElem));
};


}

#endif
