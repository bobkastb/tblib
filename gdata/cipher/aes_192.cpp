#include "blockCipher.h"

namespace tbgeneral {

#define AES_ID 192
namespace ns_AES_192 {

#include "aes_inc.hpp"
};


cCipher_Proxy Make_AES_192() {	return cCipher_Proxy::CreateNewAs<ns_AES_192::AES_ctx>(); }

}; // namespace tbgeneral {