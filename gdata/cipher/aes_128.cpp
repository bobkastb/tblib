#include "blockCipher.h"

namespace tbgeneral {

namespace ns_AES_128 {

#define AES_ID 128
#include "aes_inc.hpp"
};

/*
c_AES_base* Make_AES_128() {
	return new ns_AES_128::AES_ctx();
}
*/


cCipher_Proxy Make_AES_128() {
	//ns_AES_128::AES_ctx dd();
	//return cCipher_Proxy( dd );
	return cCipher_Proxy::CreateNewAs<ns_AES_128::AES_ctx>();
	//return * (cCipher_Proxy*) & rc_base_struct<ns_AES_128::AES_ctx>::CreateNew()
}


}; // namespace tbgeneral {