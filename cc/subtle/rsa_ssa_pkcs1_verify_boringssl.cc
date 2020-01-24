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

#include "tink/subtle/rsa_ssa_pkcs1_verify_boringssl.h"
#include "absl/strings/str_cat.h"
#include "openssl/bn.h"
#include "openssl/digest.h"
#include "openssl/evp.h"
#include "openssl/rsa.h"
#include "tink/subtle/common_enums.h"
#include "tink/subtle/subtle_util_boringssl.h"
#include "tink/util/errors.h"

namespace crypto {
namespace tink {
namespace subtle {

// static
util::StatusOr<std::unique_ptr<RsaSsaPkcs1VerifyBoringSsl>>
RsaSsaPkcs1VerifyBoringSsl::New(
    const SubtleUtilBoringSSL::RsaPublicKey& pub_key,
    const SubtleUtilBoringSSL::RsaSsaPkcs1Params& params) {
  // Check hash.
  auto hash_status =
      SubtleUtilBoringSSL::ValidateSignatureHash(params.sig_hash);
  if (!hash_status.ok()) {
    return hash_status;
  }
  auto sig_hash_result = SubtleUtilBoringSSL::EvpHash(params.sig_hash);
  if (!sig_hash_result.ok()) return sig_hash_result.status();

  // Check RSA's modulus.
  auto status_or_n = SubtleUtilBoringSSL::str2bn(pub_key.n);
  if (!status_or_n.ok()) return status_or_n.status();
  auto status_or_e = SubtleUtilBoringSSL::str2bn(pub_key.e);
  if (!status_or_e.ok()) return status_or_e.status();
  size_t modulus_size = BN_num_bits(status_or_n.ValueOrDie().get());
  if (modulus_size < kMinModulusSizeInBits) {
    return ToStatusF(
        util::error::INVALID_ARGUMENT,
        "Modulus size is %zu; only modulus size >= %zu-bit is supported",
        modulus_size, kMinModulusSizeInBits);
  }
  bssl::UniquePtr<RSA> rsa(RSA_new());
  if (rsa.get() == nullptr) {
    return util::Status(util::error::INTERNAL,
                        "BoringSsl RSA allocation error");
  }
  // Set RSA public key and hence d is nullptr.
  if (1 != RSA_set0_key(rsa.get(), status_or_n.ValueOrDie().get(),
                        status_or_e.ValueOrDie().get(), nullptr /* d */)) {
    return util::Status(util::error::INTERNAL, "Could not set RSA key.");
  }
  status_or_n.ValueOrDie().release();
  status_or_e.ValueOrDie().release();
  std::unique_ptr<RsaSsaPkcs1VerifyBoringSsl> verify(
      new RsaSsaPkcs1VerifyBoringSsl(std::move(rsa),
                                     sig_hash_result.ValueOrDie()));
  return std::move(verify);
}

RsaSsaPkcs1VerifyBoringSsl::RsaSsaPkcs1VerifyBoringSsl(bssl::UniquePtr<RSA> rsa,
                                                       const EVP_MD* sig_hash)
    : rsa_(std::move(rsa)), sig_hash_(sig_hash) {}

util::Status RsaSsaPkcs1VerifyBoringSsl::Verify(absl::string_view signature,
                                                absl::string_view data) const {
  // BoringSSL expects a non-null pointer for data,
  // regardless of whether the size is 0.
  data = SubtleUtilBoringSSL::EnsureNonNull(data);

  auto digest_result = boringssl::ComputeHash(data, *sig_hash_);
  if (!digest_result.ok()) return digest_result.status();
  auto digest = std::move(digest_result.ValueOrDie());

  if (1 !=
      RSA_verify(EVP_MD_type(sig_hash_),
                 /*msg=*/digest.data(),
                 /*msg_len=*/digest.size(),
                 /*sig=*/reinterpret_cast<const uint8_t*>(signature.data()),
                 /*sig_len=*/signature.length(),
                 /*rsa=*/rsa_.get())) {
    // Signature is invalid.
    return util::Status(util::error::UNKNOWN, "Signature is not valid.");
  }

  return util::Status::OK;
}

}  // namespace subtle
}  // namespace tink
}  // namespace crypto
