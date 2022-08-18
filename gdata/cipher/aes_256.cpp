#include "blockCipher.h"

namespace tbgeneral {

#define AES_ID 256
namespace ns_AES_256 {

#include "aes_inc.hpp"
};

cCipher_Proxy Make_AES_256() { return cCipher_Proxy::CreateNewAs<ns_AES_256::AES_ctx>(); }

}; // namespace tbgeneral {