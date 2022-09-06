#pragma once

#include "gdata/tb_algo.h"
#include "gdata/t_string.h"
#include "gdata/t_sstruct.h"

// примеры и тесты: ./test/test_tblib/test_chifer.cpp

namespace tbgeneral {

	struct cCipher_Base;
	typedef tbgeneral::rc_base_struct<cCipher_Base> cCipher_Proxy;


	struct cChipherKey {
		darray<byte> resA;
		stringA resS;
		//cChipherKey(const byte * asB , int sz) { resA. ; };
		cChipherKey(const darray<byte> & asA) { resA = asA; };
		cChipherKey(const byte * asA, uint sz) { resA.assign( asA , sz); };
		cChipherKey(const stringA & asS) { resS = asS; };
		cChipherKey(const char* asS) { resS = asS; };
		cChipherKey(const char * & asS) { resS = asS; };
		cChipherKey(const darray<byte> & asA, int bytekeysz) { resA = asA; init(bytekeysz);	};
		cChipherKey(const stringA & asS, int bytekeysz) { resS = asS; init(bytekeysz); };
		cChipherKey(const char * & asS, int bytekeysz) { resS = asS; init(bytekeysz); };
		cChipherKey() {}
		static cChipherKey random(int sz);
		void init(int sz);
		size_t getkeysize() const { return resA.size();  }
	};

	struct cCipher_Base {
		//darray<byte> fbuff;
		darray<byte> startIv;
		virtual void start() {};
		virtual void encode(void * data, size_t sz) {};
		virtual void decode(void * data, size_t sz) {};
		virtual void end() {};
		virtual const char* cipherID() { return 0; };
		static cCipher_Proxy Make(int hid);
		// full_encode(darray<byte>) может выравнивать размер массива если того требует блочный шифр
		void full_encode(darray<byte> & data ); 
		// full_encode выдает exception если размер массива не выровнен на размер блока шифра
		void full_encode(void * data, size_t sz);
		// full_decode выдает exception если размер массива не выровнен на размер блока шифра
		void full_decode(void * data, size_t sz);
		darray<byte> nfull_decode(const void * data, size_t sz) { 
			darray<byte> nd( (const byte*)data,sz); full_decode(nd.data(), nd.size()); return nd; }
		void nfull_decode(darray<byte> & out , const void * data, size_t sz) { 
			out.assign( (const byte*)data,sz); full_decode(out.data(), out.size()); }

		void alignbuffer(darray<byte> & buff);
		virtual int needAlignSize() { return 0; };
		virtual int getblocksize() { return 0; };
		virtual int getIVsize() { return 0; };
		virtual void setIv(const darray<byte> & iv) {};
		virtual int getkeysize(){ return 0; };
	};
	


	struct c_AES_base : public cCipher_Base {
		enum eMethod { 
			mECB, // блочное шифрование без IV
			mCBC, // блочное шифрование с IV
			mCTR // разрешает размер данных не кратный размеру блока шифрования
		};
		enum eKeySize { AES_128 = 128, AES_192 = 192, AES_256 = 256 };
		enum eKeySizeB { bAES_128 = 16, bAES_192 = AES_192/8, bAES_256 = AES_256/8 };
		enum { AES_BLOCKLEN = 16 };
		eMethod method;
		uint8 Iv[AES_BLOCKLEN];
		virtual void start() override;
		virtual void encode(void * data, size_t sz) override;
		virtual void decode(void * data, size_t sz) override;
		void setIv(const darray<byte> & iv) override;


		static cCipher_Proxy Make(eMethod _method, const cChipherKey & key, const darray<byte> & iv);
		static cCipher_Proxy Make(eMethod _method, const cChipherKey & key);
		int getblocksize() { return AES_BLOCKLEN; }; // virtual 
		int getIVsize() { return AES_BLOCKLEN; };

		// prvate
		typedef uint8 state_t[4][4];

		void CBC_encrypt_buffer(uint8* buf, size_t length);
		void CBC_decript_buffer(uint8* buf, size_t length);
		void ECB_encrypt_buffer(uint8* buf, size_t length) const;
		void ECB_decript_buffer(uint8* buf, size_t length) const;
		void CTR_xcrypt_buffer(uint8* buf, size_t length);
		void XorWithIv(uint8* buf, const uint8* Iv) const;
		void init(const uint8* key, const uint8* iv);
		virtual int needAlignSize() override;

		//-------------------
		//typedef uint8 state_t[4][4];

		//uint8 RoundKey[AES_keyExpSize];
		//uint8 Iv[AES_BLOCKLEN];
		//int getkeysize() =0;override;
		virtual void KeyExpansion(const uint8* Key)=0;
		virtual void Cipher(state_t* state)  const=0;
		virtual void InvCipher(state_t* state) const=0; // const uint8* RoundKey 




	};
	//enum eCipherID { cid_AES_ }

	struct c_Transposition_Chifer : public cCipher_Base {
		private:
		cChipherKey fKey;		
		public:
		virtual void start() override;
		virtual void encode(void * data, size_t sz) override;
		virtual void decode(void * data, size_t sz) override;
		void setIv(const darray<byte> & iv) override {} ;

		void init( const cChipherKey & key );
		c_Transposition_Chifer() {};
		c_Transposition_Chifer(const cChipherKey & key) { init(key); }
	};
	

	struct c_transposition {
		static cChipherKey preparekey(const cChipherKey & key);
		static darray<int> generate( const cChipherKey & key , size_t count );
		template<class TData> static void make( const cChipherKey & _key , TData * data, size_t count ){
			auto key= preparekey(_key);
			auto ikeyptr= key.resA.data(); auto ekeyptr=&key.resA[key.resA.size()-4];
			auto kp = ikeyptr;
			for(size_t i=0;i<count;i++,kp++) { 
				auto nsz= count-i;
				if (kp>ekeyptr) kp=ikeyptr;
				size_t j= (*(int*)kp) % nsz;
				//auto t=data[i]; data[i]=data[i+j]; data[i+j]=t;
				swap_lr<TData>( data[i], data[i+j] );
			}
		};
	};


}; // namespace tbgeneral
