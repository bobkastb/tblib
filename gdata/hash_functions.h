#pragma once

#include "gdata/t_string.h"
#include "gdata/t_sstruct.h"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wmultichar"
#endif



namespace tbgeneral{

struct cHash_Base;
typedef tbgeneral::rc_base_struct<cHash_Base> cHash_Proxy;

struct cHash_Base{
	//using stringA = rc_string<char>; //TODO:

	enum HashClassID{ MD5=0x004d4435, //'MD5', 
		SHA256= 0x53323536 // 'S256' 
	};
	enum { SHA256_SIZE= 256/8 , MD5_SIZE=16 };
	darray<byte> hResult , calcBuff;
	virtual void start() {};
	virtual void calc(const void * data , int sz){};
	virtual void end(){};
	virtual const char* hashID(){ return 0;};
	static cHash_Proxy MakeHash( int hid );
	static stringA Hash2Str(const darray<byte> & d);
	stringA Hash2Str(){ return Hash2Str(hResult); };
	darray<byte> CalcHash(const void * d, size_t sz);
	stringA CalcHashS(const void * d, size_t sz);
	stringA CalcHashS(const stringA & s) { return CalcHashS(s.cbegin(),  s.size()  );  };
	stringA CalcHashS(const char * s) { return CalcHashS(s,  strlen_s(s)); };
	static darray<byte> CalcHash(int hashid, const void * d, size_t sz);
	static stringA		CalcHashS(int hashid, const void * d, size_t sz){ return Hash2Str( CalcHash( hashid , d , sz )); };
	static darray<byte> CalcHash(int hashid, const stringA & s ) { return CalcHash( hashid , s.data(), s.size() );};
	static stringA CalcHashS(int hashid, const stringA & s ) { return Hash2Str( CalcHash( hashid , s.data(), s.size() ));};
	static darray<byte> CalcHashFile(int hashid, const stringA & filename );
	static stringA CalcHashFileS(int hashid, const stringA & filename ) { return Hash2Str( CalcHashFile(hashid, filename )); };

	static void CalcN(int hashid, darray<byte> & d , size_t count=1);
	static darray<byte> CalcNOut(int hashid, const void * d, size_t dsize, size_t cnt=1);
	static darray<byte> CalcNOut(int hashid, const darray<byte> & d, size_t cnt=1) {
		return CalcNOut(hashid , d.data(), d.size() , cnt );};
	static void CalcGenSize(int hashid, darray<byte> & d, size_t finsize);
	static darray<byte> makeNhash(int hashid, const void * indata , uint szdata , uint szout , int count=1);
};

template<class TString> darray<byte> hash_StrNHash( int hashid, const TString & indata , uint szout , int count=1 ){
	return cHash_Base::makeNhash(hashid , indata.data() , indata.size() , szout , count );
}




}; // namespace tbgeneral