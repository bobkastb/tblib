#include "blockCipher.h"

namespace tbgeneral {


void cCipher_Base::full_encode(void* data, size_t sz) {
	auto na = needAlignSize();
	if (na!=0 && (sz % na)!=0 ) throw "cCipher_Base not aligned size for block chifer";
	start();
	encode(data, sz); 
	end(); }
void cCipher_Base::full_decode(void* data, size_t sz) {
	auto na = needAlignSize();
	if (na != 0 && (sz % na) != 0) throw "cCipher_Base not aligned size for block chifer";
	start(); decode(data, sz); end(); }
void cCipher_Base::full_encode(darray<byte>& data){
	auto na = needAlignSize();
	auto rem = data.size() % na;
	if (na != 0 && rem != 0) {
		data.resize(data.size()+na-rem);
	};
	full_encode(data.begin(), data.size());

};

void cCipher_Base::alignbuffer(darray<byte> & buff) {
	auto bsz = getblocksize();
	auto csz = buff.size();
	auto rem = csz % bsz;
	if (!rem) { return; };
	auto asz = bsz - rem;
	buff.resize(csz + asz);
	memset(&buff[csz], 0, asz);
};



cCipher_Proxy c_AES_base::Make(eMethod _method, const cChipherKey & key) {
	static byte DefaultAesIVdata[AES_BLOCKLEN] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
	static darray<byte> dba(DefaultAesIVdata, AES_BLOCKLEN);
	return Make(_method, key, dba);
};




void c_AES_base::XorWithIv(uint8* buf, const uint8* Iv) const	{
		uint8 i;
		for (i = 0; i < c_AES_base::AES_BLOCKLEN; ++i) // The block in AES is always 128bit no matter the key size
		{
			buf[i] ^= Iv[i];
		}
}


void c_AES_base::CBC_encrypt_buffer(uint8* buf, size_t length){
	uintptr_t i;
	const uint8 *Iv_n = this->Iv;
	for (i = 0; i < length; i += AES_BLOCKLEN)
	{
		XorWithIv(buf, Iv_n);
		this->Cipher((state_t*)buf);
		Iv_n = buf;
		buf += AES_BLOCKLEN;
		//printf("Step %d - %d", i/16, i);
	}
	/* store Iv in ctx for next call */
	memcpy(this->Iv, Iv, AES_BLOCKLEN);
};

void c_AES_base::CBC_decript_buffer(uint8* buf, size_t length){
	uintptr_t i;
	uint8 storeNextIv[AES_BLOCKLEN];
	for (i = 0; i < length; i += AES_BLOCKLEN)
	{
		memcpy(storeNextIv, buf, AES_BLOCKLEN);
		this->InvCipher((state_t*)buf);
		XorWithIv(buf, this->Iv);
		memcpy(this->Iv, storeNextIv, AES_BLOCKLEN);
		buf += AES_BLOCKLEN;
	}
};

void c_AES_base::ECB_encrypt_buffer(uint8* buf, size_t length) const{
	// The next function call encrypts the PlainText with the Key using AES algorithm.
	for (size_t i = 0; i < length; i += AES_BLOCKLEN, buf += AES_BLOCKLEN) {
		//this->Cipher((state_t*)buf, this->RoundKey);
		this->Cipher((state_t*)buf);
	}

};
void c_AES_base::ECB_decript_buffer(uint8* buf, size_t length) const {
	// The next function call decrypts the PlainText with the Key using AES algorithm.
	for (size_t i = 0; i < length; i += AES_BLOCKLEN, buf += AES_BLOCKLEN) {
		//this->InvCipher((state_t*)buf, ctx->RoundKey);
		this->InvCipher((state_t*)buf);
	}
};

void c_AES_base::CTR_xcrypt_buffer(uint8* buf, size_t length) {
	/* Symmetrical operation: same function for encrypting as for decrypting. Note any IV/nonce should never be reused with the same key */
	uint8 buffer[AES_BLOCKLEN];

	size_t i;
	int bi;
	for (i = 0, bi = AES_BLOCKLEN; i < length; ++i, ++bi)
	{
		if (bi == AES_BLOCKLEN) /* we need to regen xor compliment in buffer */
		{

			memcpy(buffer, this->Iv, AES_BLOCKLEN);
			this->Cipher((state_t*)buffer);

			/* Increment Iv and handle overflow */
			for (bi = (AES_BLOCKLEN - 1); bi >= 0; --bi)
			{
				/* inc will overflow */
				if (this->Iv[bi] == 255)
				{
					this->Iv[bi] = 0;
					continue;
				}
				this->Iv[bi] += 1;
				break;
			}
			bi = 0;
		}

		buf[i] = (buf[i] ^ buffer[bi]);
	}
};



void c_AES_base::setIv(const darray<byte> & iv) {
	if (iv.size() != sizeof(Iv)) { throw "c_AES_base::setIv: Invalid iv.size()"; }
	memcpy(Iv, iv.data(), sizeof(Iv));
};

int c_AES_base::needAlignSize() {
	switch (this->method) {
	case mCBC:	
	case mECB:	return getblocksize();
	case mCTR:	return 0;
	}
	return getblocksize();
}
void c_AES_base::encode(void * data, size_t sz) {
	switch (this->method) {
	case mCBC:	CBC_encrypt_buffer((byte*)data, sz); break;
	case mECB:	ECB_encrypt_buffer((byte*)data, sz); break;
	case mCTR:	CTR_xcrypt_buffer((byte*)data, sz); break;
	}
};
void c_AES_base::decode(void * data, size_t sz) {
	switch (this->method) {
	case mCBC:	CBC_decript_buffer((byte*)data, sz); break;
	case mECB:	ECB_decript_buffer((byte*)data, sz); break;
	case mCTR:	CTR_xcrypt_buffer((byte*)data, sz); break;
	}
};


void c_AES_base::init(const uint8* key, const uint8* iv) {
		this->KeyExpansion(key);
		memcpy(this->Iv, iv, AES_BLOCKLEN);
		startIv = darray<byte>(iv,sizeof(this->Iv));
};

void c_AES_base::start(){
	setIv( startIv );
};


cCipher_Proxy Make_AES_256();
cCipher_Proxy Make_AES_128();
cCipher_Proxy Make_AES_192();


cCipher_Proxy c_AES_base::Make(eMethod _method, const cChipherKey & key, const darray<byte> & iv) {
	if (iv.size() != AES_BLOCKLEN) { throw("Invalid len Iv for AES!"); }
	auto ksz = key.getkeysize();
	//c_AES_base* 
	cCipher_Proxy ncip;
	switch (ksz) {
	case bAES_128: ncip = Make_AES_128(); break;
	case bAES_192: ncip = Make_AES_192(); break;
	case bAES_256: ncip = Make_AES_256(); break;
	default: throw("Invalid key size for AES!");
	}
	auto cbase = (c_AES_base*)ncip.fdata;
	cbase->method = _method;
	cbase->init(key.resA.data(), iv.data());
	return ncip;
};


}; // namespace tbgeneral



#include "gdata/hash_functions.h"
namespace tbgeneral{
void cChipherKey::init(int sz) {
	byte defkey[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
	if (resA.size() == 0) { 
		if (resS.size() == 0) {
			resA.assign(defkey, sizeof(defkey));
		} else 	resA.assign( (const byte*) resS.data(), resS.size()); }
	if ((int)resA.size() > sz) { resA.resize(sz);  }
	if (resA.size() == size_t(sz)) { return; }
	cHash_Base::CalcGenSize(cHash_Base::MD5, resA, sz);
};
}; // namespace tbgeneral

