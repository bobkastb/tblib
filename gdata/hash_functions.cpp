#include "hash_functions.h"
#include "tbt_locale.h"
#include "t_format.h"
#include "tb_numbers.h"
#include "tsys/tbfiles.h"

#define INC_HEADERS_ONLY
#include "hashes/md5.cpp"


namespace tbgeneral{
struct cHash_MD5;

cHash_Proxy Make_Sha256();
cHash_Proxy cHash_Base::MakeHash( int hid ){
    switch (hid) {
		case MD5 : return rc_base_struct<cHash_Base>::CreateNewAs<cHash_MD5>();
		case SHA256 : return Make_Sha256();
	};
	return cHash_Proxy();
};

stringA cHash_Base::Hash2Str(const darray<byte> & d){ 
	return arr2hex(d ,true );
};

darray<byte> cHash_Base::CalcHash(const void * d, size_t sz) {
	this->start();
	this->calc((void*)d, static_cast<int>( sz) );
	this->end();
	return hResult;
};
stringA cHash_Base::CalcHashS(const void * d, size_t sz) {
	this->CalcHash(d, sz);
	return this->Hash2Str();
};




struct cHash_MD5 : public cHash_Base{
	ns_hashes_md5::MD5_CTX ctx;
	virtual const char* hashID(){ return "MD5"; };
	virtual void start(){ this->hResult.resize(16);
			ns_hashes_md5::MD5Init( &ctx );
	 };
	virtual void calc(const void * data , int sz){
			ns_hashes_md5::MD5Update( &ctx , (byte*) data , sz );	
	};
	virtual void end(){
			hResult.resize(16);
			ns_hashes_md5::MD5Final( hResult.data() , &ctx );
	};
};


darray<byte> cHash_Base::CalcHash(int hashid, const void * d, size_t sz) {
	auto h = cHash_Base::MakeHash(hashid);
	return h->CalcHash(d, sz);
};

void cHash_Base::CalcN(int hashid, darray<byte> & d, size_t count) {
	auto h = cHash_Base::MakeHash(hashid);
	auto r = h->hResult;
	for (size_t i = 0; i < count; i++) {
		r=h->CalcHash(d.data(), d.size() );
		d.assign(r.data(), r.size());
	}
};

void cHash_Base::CalcGenSize(int hashid, darray<byte> & d, size_t finsize) {
	auto h = cHash_Base::MakeHash(hashid);
	auto r = h->hResult;
	d.reserve(finsize);
	for (int i = 0; (i < 1) || int(d.size())<finsize; i++) {
		r = h->CalcHash(d.data(), d.size());
		if (i == 0) {
			d.assign(r.data(), r.size());
		} else {
			d.append(r);
		}
	}
	d.resize(finsize);
};

darray<byte> cHash_Base::makeNhash(int hashid, const void * indata , uint szdata , uint szout , int count){
	darray<byte> result; result.resize(szout); size_t respos=0; 
	auto h = cHash_Base::MakeHash(hashid);
	//auto r = h->hResult;
	darray<byte> r((const byte*)indata, szdata);
	auto resptr= result.data(); size_t ressize= result.size();
	for (int i = 0; (i < count) ; i++) {
		r = h->CalcHash(r.data(), r.size());
		if (ressize > r.size()) {
		for( size_t rpos=0,rsize=r.size(); rpos < rsize;  ) {
			auto csz= std::min( ressize - respos , rsize - rpos );
			memcpy( resptr+respos , r.data(), csz );
			respos = (respos + csz) % ressize; rpos+=csz; 
		}} else {
			memcpy( resptr , r.data(), ressize );
		}
	}
	return result;
};
darray<byte> cHash_Base::CalcNOut(int hashid, const void * d, size_t dsize, size_t cnt){
	auto h = cHash_Base::MakeHash(hashid);
	auto r = h->CalcHash(d, dsize);
	for(;cnt;cnt--) r=h->CalcHash(r.data(), r.size());
	return r;
};

darray<byte> cHash_Base::CalcHashFile(int hashid, const stringA & filename ){
	auto data = tbgeneral::filesystem::readstrfromfile( filename );
	if (!data.size()) { return darray<byte>(); } //error
	return cHash_Base::CalcHash(hashid, data.data(), data.size() );
};


//---------------------
class cHASHStream {
public:
	enum { eStartVal = 2166136261U, eStepVal = 16777619U };
	typedef size_t basetype;
	template<class TChar> struct AlgoTrivial { static TChar fun(const TChar & s) { return s; } };
	template<class TChar, class Algo> static basetype calc(const TChar * s, const TChar * e) {
		size_t _Val = eStartVal;
		for (; s<e; s++) {
			_Val = eStepVal * _Val ^ (size_t)Algo::fun(*s);
		};
		return _Val;
	}
};

size_t DefHash4_value_stream(size_t Val, const void * begin, const void * end) {
	auto bs = (const byte*)begin, be = (const byte*)end;
	for (; bs<be; bs++) {
		Val = cHASHStream::eStepVal * Val ^ bs[0];
	};
	return Val;
}

size_t DefHash4_value(const void * begin, const void * end) {
	return DefHash4_value_stream(cHASHStream::eStartVal, begin, end);

};

template<typename TChar > size_t icase_DefHash4_value_char_t(const TChar * s, const TChar * e) {
		// class Algo { static TChar fun(TChar) { return tbgeneral::f_chartoupper( *s )} };
		enum { BufferLen = 2048 };
		codec_case_t<TChar> ccd;
		size_t _Val = cHASHStream::eStartVal;
		char buff[BufferLen];
		auto cs = s;
		for (; cs<e; s++) {
			auto cc = cs;
			auto cbytes = ccd.caser.prepareforHashIcase(buff, sizeof(buff), cs, e, cc);
			if (cbytes == 0) { throw 1; }
			cs = cc;
			_Val = DefHash4_value_stream(_Val, buff, buff + cbytes);
		};
		return _Val;
}

size_t icase_DefHash4_value(const wchar_t * s, const wchar_t * e) { 
	return icase_DefHash4_value_char_t(s, e);
};
size_t icase_DefHash4_value(const char * s, const char * e) {
	return icase_DefHash4_value_char_t(s, e);
};
/*
template<class TChar > size_t icase_hash_value(const TChar * s, const TChar * e)
{	// class Algo { static TChar fun(TChar) { return tbgeneral::f_chartoupper( *s )} };
	size_t _Val = cHASHStream::eStartVal;
	for (; s<e; s++) {
		//_Val = cHASHStream::eStepVal * _Val ^ ((size_t)tbgeneral::f_chartoupper( *s )) ;
	};
	return _Val;
}
template<class TChar > size_t case_hash_value(const TChar * s, const TChar * e)
{
	size_t _Val = cHASHStream::eStartVal;
	for (; s<e; s++) {
		_Val = cHASHStream::eStepVal * _Val ^ ((size_t)(*s));
	};
	return _Val;
}
*/
//---------------------

};

