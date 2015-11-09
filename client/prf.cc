#include "audit/client/prf.h"

#include "cryptopp/integer.h"
#include "cryptopp/hmac.h"
#include "cryptopp/sha.h"
#include "openssl/bn.h"

#include "audit/common.h"
#include "audit/cpor_types.h"

namespace audit {

BN_ptr SiphashPRF::Encode(unsigned int i) {
  hmac_.Restart();

  unsigned char bytes[4];
  bytes[0] = (i >> 24) & 0xFF;
  bytes[1] = (i >> 16) & 0xFF;
  bytes[2] = (i >> 8) & 0xFF;
  bytes[3] = i & 0xFF;

  unsigned char digest[hmac_.DigestSize()];
  hmac_.Update(&bytes[0], 4);
  hmac_.Final(&digest[0]);

  BN_ptr result{BN_new(), ::BN_free};
  BN_bin2bn(&digest[0], hmac_.DigestSize(), result.get());

  return std::move(result);
}
}