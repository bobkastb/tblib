#pragma once

#include "gdata/t_string.h"


namespace tbgeneral{


struct t_bitstream{
	darray<byte> bitstream; 
	uint countbits_in; 
	
	void push(int v);
	int pop();
	bool eof();
	bool eof(int delta);
	t_bitstream() {countbits_in=0;}
	//void seekstart() { countbits_in=0; }
	darray<byte> data() { return bitstream; }
	void finish_push();
	void prepare_push( uint sz );
	void prepare_pop( const darray<byte> & d );

};














struct bufferization_string__bad { 
	using pstring = stringA;
	using t_string = stringA;

	pstring buff;
	int flock;
	size_t lockpos;

	bufferization_string__bad() { lockpos=0;flock=0; }
	size_t lock( )  {
		if (flock) throw "buffer is locked!";
		lockpos = buff.size();
		flock++;
		return lockpos;
	}
	pstring unlock() {
		if (flock<=0) { throw "buffer is NO locked!"; }
		flock--;
		return buff.slice(lockpos);
	}
	void make( size_t bsize ) { 
		if (flock>0) {bsize=bsize*5/4;}
		buff.reserve(bsize);
	}
	size_t size() { return buff.size(); }
	pstring push( const char * _s , size_t sz ){
		auto p = buff.size();
		buff.append( _s, sz );
		return buff.slice( p );

	} 
	pstring push( const pstring & sd  ){ return push( sd.cbegin(), sd.size() ); }
};

}