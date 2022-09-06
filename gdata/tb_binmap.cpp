#include "tb_basetypes.h"
#include <algorithm>
#include "tb_binmap.h"
#include "stdio.h"

namespace tbgeneral{ 

int c_MapBin::inverseto( c_MapBin & res , bool clear){
	if (clear) res.init( startpoint , this->maxsize , this->predefRangeCnt );
	int st=this->startpoint;
	for (iterator ii=begin(); ii<end() ; ii=next(ii) ) {
		if (st< ii.start) res.pushbackRange( st, ii.start - st);
		st = ii.start + ii.sz;
	};
	if ( st< startpoint+maxsize ) res.addrange(st,startpoint+maxsize-st);
	return 0;
};


void c_MapBin_List::init(int _st,int _sz, int _predefRangeCnt){ 
	c_MapBin::init(_st , _sz, _predefRangeCnt);
	lst.reserve( _predefRangeCnt );	
	lst.resize(0); 
};

int c_MapBin_List::findlo(const cRange & r){
	for (uint i=0;i<lst.size();i++) 
		if (r.start<=lst[i].start) { return i;}
	return -1;
};  

void c_MapBin_List::pushbackRange( int start , int sz ){
	cRange r(start , sz);
    if (r.sz>0) lst.push_back(r); 
};

void c_MapBin_List::addrange( const cRange & r ){
	//int changes=0;
	int st= findlo(r); int lasti=(int)lst.size()-1;  int ucnt=0;
	cRange *pred = st>0 ? &lst[st-1] : st==0 ? 0 : lasti>=0 ? &lst[lasti] : 0;
	cRange *nxt = st>=0 ? &lst[st] : 0 ;
	cRange * stp= nxt;
	if (pred!=0 && pred->isintersectWP(r)) { *pred=pred->dounion(r); ucnt++; }
	if (stp!=0) { 
		cRange *n=stp , *l= lst.data()+lasti+1; 
		for (;(n<l) && (n->isintersectWP(r));n++, ucnt++)
			*stp=n->dounion(r); 
	};
	switch (ucnt) {
		case 0:{
			if (st<0) lst.push_back(r);	
			else vinsert( lst ,st , r );	
			}; break;
		case 1:break; 	
		default: { vdelete(lst , stp+1-lst.data() , ucnt-1);	};
	};
	
};
int c_MapBin_List::findAt( int start ){
	for (uint i=0;i<lst.size();i++) { int v=start-lst[i].start;
		if (v<0) return false;
		if (v<lst[i].sz) return true;
	}
	return false;
}; 
int c_MapBin_List::power(){
	int res=0; 
	for (uint i=0;i<lst.size();i++) { res+= lst[i].sz; };
	return res;
}; 
int c_MapBin_List::findrange( const cRange & r ) // 0 out , 1-left part  2-right part ,3-midle , 7- full
{
	//findlo
	return -1;
};


c_MapBin_List::iterator c_MapBin_List::next(const iterator& it){
    if (it.index<0) return it;
	if (it.index+1 < int(lst.size()) ) { return iterator(it.index+1, lst[it.index+1]); }
	else return end();
};

bool c_MapBin::cRange::isintersect (const cRange& r) const { 
	return isintersect(r.start) || r.isintersect(start) ;
};

bool c_MapBin::cRange::isintersectWP (const cRange& r) const {// c границами
	return isintersectWHP(r.start) ||  r.isintersectWHP(start);
}; 
c_MapBin::cRange c_MapBin::cRange::intersect (const cRange& r) const {
	if (!isintersect(r)) return cRange();
	int l=std::max(start,r.start),h=std::min(start+sz,r.start+r.sz);
	return cRange( l , h-l );
} ;
c_MapBin::cRange c_MapBin::cRange::dounion (const cRange& r) const {
	int l=std::min(start,r.start),h=std::max(start+sz,r.start+r.sz);
	return cRange( l , h-l );
} ;

//*******************

namespace test {

template <class CMap> void PRINTMAP(CMap & map) { 
	printf(">"); 
	for (auto ii=map.begin(); ii< map.end(); ii=map.next(ii) ) 
		printf("[%d %d] " , ii.start , ii.start+ii.sz-1);	
	printf("\n"); 
};

void test_c_MapBin_List(){
	//int k=0;
	c_MapBin_List map;
	c_MapBin_List mapi;
	map.init(0,128*1024,0x1000);
	for (int i=0;i<10;i++) {
		map.addrange( c_MapBin::cRange( i*100, 10));
	};
	PRINTMAP(map);
	map.addrange( c_MapBin::cRange( 1*100, 10));
	PRINTMAP(map);
	for (int i=0;i<10;i++) {	map.addrange( c_MapBin::cRange( i*100, 10));	};
	PRINTMAP(map);
	map.addrange( c_MapBin::cRange( 25, 300));
	PRINTMAP(map);
	map.inverseto( mapi , true );
	PRINTMAP(mapi);
	//k=0;
};
} // namespace test {

}; // namespace