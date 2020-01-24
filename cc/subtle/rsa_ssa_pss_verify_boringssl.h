// Copyright 2018 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef THIRD_PARTY_TINK_CC_SUBTLE_RSA_SSA_PSS_VERIFY_BORINGSSL_H_
#define THIRD_PARTY_TINK_CC_SUBTLE_RSA_SSA_PSS_VERIFY_BORINGSSL_H_

#include <memory>

#include "absl/strings/string_view.h"
#include "openssl/evp.h"
#include "openssl/rsa.h"
#include "tink/public_key_verify.h"
#include "tink/subtle/common_enums.h"
#include "tink/subtle/subtle_util_boringssl.h"
#include "tink/util/status.h"

namespace crypto {
namespace tink {
namespace subtle {

// RSA SSA (Signature Schemes with Appendix) using  PSS  (Probabilistic
// Signature Scheme) encoding is defined at
// https://tools.ietf.org/html/rfc8017#section-8.1). This implemention uses
// Boring SSL for the underlying cryptographic operations.
class RsaSsaPssVerifyBoringSsl : public PublicKeyVerify {
 public:
  static crypto::tink::util::StatusOr<std::unique_ptr<RsaSsaPssVerifyBoringSsl>>
  New(const SubtleUtilBoringSSL::RsaPublicKey& pub_key,
      const SubtleUtilBoringSSL::RsaSsaPssParams& params);

  // Verifies that 'signature' is a digital signature for 'data'.
  crypto::tink::util::Status Verify(absl::string_view signature,
                                    absl::string_view data) const override;

  ~RsaSsaPssVerifyBoringSsl() override = default;

 private:
  // To reach 128-bit security strength, RSA's modulus must be at least 3072-bit
  // while 2048-bit RSA key only has 112-bit security. Nevertheless, a 2048-bit
  // RSA key is considered safe by NIST until 2030 (see
  // https://www.keylength.com/en/4/).
  static const size_t kMinModulusSizeInBits = 2048;
  RsaSsaPssVerifyBoringSsl(bssl::UniquePtr<RSA> rsa, const EVP_MD* sig_hash,
                           const EVP_MD* mgf1_hash, int salt_length);
  const bssl::UniquePtr<RSA> rsa_;
  const EVP_MD* sig_hash_;   // Owned by BoringSSL.
  const EVP_MD* mgf1_hash_;  // Owned by BoringSSL.
  int salt_length_;
};

}  // namespace subtle
}  // namespace tink
}  // namespace crypto

#endif  // THIRD_PARTY_TINK_CC_SUBTLE_RSA_SSA_PSS_VERIFY_BORINGSSL_H_
