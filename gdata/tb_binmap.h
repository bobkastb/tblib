#pragma once

#include "tb_basetypes.h"
#include <vector>

namespace tbgeneral{ 

class c_MapBin{
	public:
	int startpoint,maxsize;
	int predefRangeCnt;
	public:
		struct cRange{ int start, sz; cRange(){start=sz=0;}; cRange(int s,int l){start=s;sz=l<0?0:l;};
			bool isintersect (const cRange& r) const ;
			bool isintersect (int pnt) const { return (pnt>=start && pnt<start+sz); };
			bool isintersectWHP (int pnt) const { return (pnt>=start && pnt<=start+sz); };
			bool isintersectWP (const cRange& r) const ; // c границами
			cRange intersect (const cRange& r) const ;
			cRange dounion (const cRange& r) const ;
			bool isempty() { return sz<=0;}
		};
		struct iterator: public cRange{
			int index;
			iterator() { index=0; }
			iterator(int _i) { index=_i; start=sz=0; }
			iterator(int _i, const cRange& r) { index=_i; *(cRange*)this=r; };
			bool isend() { return index<0;}
			bool operator == (const iterator & n) const { return index==n.index; };
			bool operator < (const iterator & n) const { return uint(index)<uint(n.index); };
		};

	virtual void init(int _st, int _sz, int _predefRangeCnt){ startpoint=_st; maxsize=_sz; predefRangeCnt= _predefRangeCnt; };
	virtual void addrange( const cRange & r )=0;
	virtual int findrange( const cRange & r )=0; // 0 out , 1-left part  2-right part ,3-midle , 7- full
	virtual void pushbackRange( int start , int sz ){ addrange(cRange(start , sz)); };
	void addrange( int start , int sz ) { addrange(cRange(start , sz)); };
	int findrange( int start , int sz ) { return findrange(cRange(start , sz)); }; 
	virtual int findAt( int start )=0; 
	virtual bool findpos( int start ){ return findAt(start)>=0;  }; 
	int size() { return maxsize; }
	virtual iterator next(const iterator& it)=0;
	virtual iterator begin()=0; 
	virtual iterator end(){ return iterator(-1); }; 
	virtual int power()=0; 
	virtual int inverseto(c_MapBin & res , bool clear=true);
	virtual size_t getRangeCount(){ return 0;};


};

class c_MapBin_List:  public c_MapBin{
	std::vector<cRange> lst;
public:
	virtual void init(int _st,int _sz, int _predefRangeCnt);
	virtual void pushbackRange( int start , int sz );
	virtual void addrange( const cRange & r );
	virtual int findrange( const cRange & r ); // 0 out , 1-left part  2-right part ,3-midle , 7- full
	virtual int findAt( int start ); 
	virtual iterator next(const iterator& it);
	virtual iterator begin() { return lst.size() ? iterator(0, lst[0]) : end(); }; 
	virtual int power(); 
	virtual size_t getRangeCount(){ return lst.size();};
protected:
    int findlo(const cRange & r);  
};

template <class V ,class E> void vinsert( V& vec , size_t st , const E& e) { 
		typename V::iterator it =vec.begin()+st;
		vec.insert( it , e );
}

template <class V > void vdelete( V& vec , size_t st , int cnt=1 ) { 
		typename V::iterator it =vec.begin()+st;
		vec.erase( it ,  it+cnt );
}

}; // namespace
