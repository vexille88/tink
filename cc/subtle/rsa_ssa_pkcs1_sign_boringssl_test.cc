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

#include "tink/subtle/rsa_ssa_pkcs1_sign_boringssl.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/strings/escaping.h"
#include "openssl/base.h"
#include "openssl/bn.h"
#include "openssl/rsa.h"
#include "tink/subtle/rsa_ssa_pkcs1_verify_boringssl.h"
#include "tink/subtle/subtle_util_boringssl.h"
#include "tink/util/test_matchers.h"

namespace crypto {
namespace tink {
namespace subtle {
namespace {
using ::crypto::tink::test::IsOk;
using ::crypto::tink::test::StatusIs;
using ::testing::IsEmpty;
using ::testing::Not;

class RsaPkcs1SignBoringsslTest : public ::testing::Test {
 public:
  RsaPkcs1SignBoringsslTest() : rsa_f4_(BN_new()) {
    EXPECT_TRUE(BN_set_u64(rsa_f4_.get(), RSA_F4));
    EXPECT_THAT(SubtleUtilBoringSSL::GetNewRsaKeyPair(
                    2048, rsa_f4_.get(), &private_key_, &public_key_),
                IsOk());
  }

 protected:
  bssl::UniquePtr<BIGNUM> rsa_f4_;
  SubtleUtilBoringSSL::RsaPrivateKey private_key_;
  SubtleUtilBoringSSL::RsaPublicKey public_key_;
};

TEST_F(RsaPkcs1SignBoringsslTest, EncodesPkcs1) {
  SubtleUtilBoringSSL::RsaSsaPkcs1Params params{/*sig_hash=*/HashType::SHA256};

  auto signer_or = RsaSsaPkcs1SignBoringSsl::New(private_key_, params);
  ASSERT_THAT(signer_or.status(), IsOk());

  auto signature_or = signer_or.ValueOrDie()->Sign("testdata");
  ASSERT_THAT(signature_or.status(), IsOk());
  EXPECT_THAT(signature_or.ValueOrDie(), Not(IsEmpty()));

  auto verifier_or = RsaSsaPkcs1VerifyBoringSsl::New(public_key_, params);
  ASSERT_THAT(verifier_or.status(), IsOk());
  EXPECT_THAT(
      verifier_or.ValueOrDie()->Verify(signature_or.ValueOrDie(), "testdata"),
      IsOk());
}

TEST_F(RsaPkcs1SignBoringsslTest, EncodesPkcs1WithSeparateHashes) {
  SubtleUtilBoringSSL::RsaSsaPkcs1Params params{/*sig_hash=*/HashType::SHA256};

  auto signer_or = RsaSsaPkcs1SignBoringSsl::New(private_key_, params);
  ASSERT_THAT(signer_or.status(), IsOk());

  auto signature_or = signer_or.ValueOrDie()->Sign("testdata");
  ASSERT_THAT(signature_or.status(), IsOk());
  EXPECT_THAT(signature_or.ValueOrDie(), Not(IsEmpty()));

  auto verifier_or = RsaSsaPkcs1VerifyBoringSsl::New(public_key_, params);
  ASSERT_THAT(verifier_or.status(), IsOk());
  EXPECT_THAT(
      verifier_or.ValueOrDie()->Verify(signature_or.ValueOrDie(), "testdata"),
      IsOk());
}

TEST_F(RsaPkcs1SignBoringsslTest, RejectsUnsafeHash) {
  SubtleUtilBoringSSL::RsaSsaPkcs1Params params{/*sig_hash=*/HashType::SHA1};
  ASSERT_THAT(RsaSsaPkcs1SignBoringSsl::New(private_key_, params).status(),
              StatusIs(util::error::INVALID_ARGUMENT));
}

TEST_F(RsaPkcs1SignBoringsslTest, RejectsInvalidCrtParams) {
  SubtleUtilBoringSSL::RsaSsaPkcs1Params params{/*sig_hash=*/HashType::SHA256};
  ASSERT_THAT(private_key_.crt, Not(IsEmpty()));
  ASSERT_THAT(private_key_.dq, Not(IsEmpty()));
  ASSERT_THAT(private_key_.dp, Not(IsEmpty()));

  // Flip a few bits in the CRT parameters; check that creation fails.
  {
    SubtleUtilBoringSSL::RsaPrivateKey key = private_key_;
    key.crt[0] ^= 0x80;
    auto signer_or = RsaSsaPkcs1SignBoringSsl::New(key, params);
    EXPECT_THAT(signer_or.status(), StatusIs(util::error::INVALID_ARGUMENT));
  }
  {
    SubtleUtilBoringSSL::RsaPrivateKey key = private_key_;
    key.dq[0] ^= 0x08;
    auto signer_or = RsaSsaPkcs1SignBoringSsl::New(key, params);
    EXPECT_THAT(signer_or.status(), StatusIs(util::error::INVALID_ARGUMENT));
  }
  {
    SubtleUtilBoringSSL::RsaPrivateKey key = private_key_;
    key.dp[0] ^= 0x04;
    auto signer_or = RsaSsaPkcs1SignBoringSsl::New(key, params);
    EXPECT_THAT(signer_or.status(), StatusIs(util::error::INVALID_ARGUMENT));
  }
}

}  // namespace
}  // namespace subtle
}  // namespace tink
}  // namespace crypto

int main(int ac, char *av[]) {
  testing::InitGoogleTest(&ac, av);
  return RUN_ALL_TESTS();
}
