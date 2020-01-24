// Copyright 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////////

#include "tink/subtle/encrypt_then_authenticate.h"

#include <string>
#include <vector>

#include "tink/subtle/aes_ctr_boringssl.h"
#include "tink/subtle/common_enums.h"
#include "tink/subtle/hmac_boringssl.h"
#include "tink/subtle/random.h"
#include "tink/util/status.h"
#include "tink/util/statusor.h"
#include "tink/util/test_util.h"
#include "gtest/gtest.h"

namespace crypto {
namespace tink {
namespace subtle {
namespace {

// Copied from
// https://tools.ietf.org/html/draft-mcgrew-aead-aes-cbc-hmac-sha2-05.
// We use CTR but the RFC uses CBC mode, so it's not possible to compare
// plaintexts. However, the tests are still valueable to ensure that we correcly
// compute HMAC over ciphertext and aad.
struct TestVector {
  std::string mac_key;
  std::string enc_key;
  std::string ciphertext;
  std::string aad;
  HashType hash_type;
  int iv_size;
  int tag_size;
};
static const std::vector<TestVector> test_vectors({
    {"000102030405060708090a0b0c0d0e0f", "101112131415161718191a1b1c1d1e1f",
     "1af38c2dc2b96ffdd86694092341bc04"
     "c80edfa32ddf39d5ef00c0b468834279"
     "a2e46a1b8049f792f76bfe54b903a9c9"
     "a94ac9b47ad2655c5f10f9aef71427e2"
     "fc6f9b3f399a221489f16362c7032336"
     "09d45ac69864e3321cf82935ac4096c8"
     "6e133314c54019e8ca7980dfa4b9cf1b"
     "384c486f3a54c51078158ee5d79de59f"
     "bd34d848b3d69550a67646344427ade5"
     "4b8851ffb598f7f80074b9473c82e2db"
     "652c3fa36b0a7c5b3219fab3a30bc1c4",
     "546865207365636f6e64207072696e63"
     "69706c65206f66204175677573746520"
     "4b6572636b686f666673",
     HashType::SHA256, 16, 16},
    {"000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f",
     "202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f",
     "1af38c2dc2b96ffdd86694092341bc04"
     "4affaaadb78c31c5da4b1b590d10ffbd"
     "3dd8d5d302423526912da037ecbcc7bd"
     "822c301dd67c373bccb584ad3e9279c2"
     "e6d12a1374b77f077553df829410446b"
     "36ebd97066296ae6427ea75c2e0846a1"
     "1a09ccf5370dc80bfecbad28c73f09b3"
     "a3b75e662a2594410ae496b2e2e6609e"
     "31e6e02cc837f053d21f37ff4f51950b"
     "be2638d09dd7a4930930806d0703b1f6"
     "4dd3b4c088a7f45c216839645b2012bf"
     "2e6269a8c56a816dbc1b267761955bc5",
     "546865207365636f6e64207072696e63"
     "69706c65206f66204175677573746520"
     "4b6572636b686f666673",
     HashType::SHA512, 16, 32},
});

static util::StatusOr<std::unique_ptr<Aead>> createAead2(
    const std::string& encryption_key, uint8_t iv_size,
    const std::string& mac_key, uint8_t tag_size, HashType hash_type) {
  auto ind_cipher_res = AesCtrBoringSsl::New(encryption_key, iv_size);
  if (!ind_cipher_res.ok()) {
    return ind_cipher_res.status();
  }

  auto mac_res = HmacBoringSsl::New(hash_type, tag_size, mac_key);
  if (!mac_res.ok()) {
    return mac_res.status();
  }
  auto cipher_res =
      EncryptThenAuthenticate::New(std::move(ind_cipher_res.ValueOrDie()),
                                   std::move(mac_res.ValueOrDie()), tag_size);
  if (!cipher_res.ok()) {
    return cipher_res.status();
  }
  return std::move(cipher_res.ValueOrDie());
}

static util::StatusOr<std::unique_ptr<Aead>> createAead(
    uint8_t encryption_key_size, uint8_t iv_size, uint8_t mac_key_size,
    uint8_t tag_size, HashType hash_type) {
  std::string encryption_key(Random::GetRandomBytes(encryption_key_size));
  std::string mac_key(Random::GetRandomBytes(mac_key_size));
  return createAead2(encryption_key, iv_size, mac_key, tag_size, hash_type);
}

TEST(AesGcmBoringSslTest, testRfcVectors) {
  for (const TestVector& test : test_vectors) {
    std::string mac_key = test::HexDecodeOrDie(test.mac_key);
    std::string enc_key = test::HexDecodeOrDie(test.enc_key);
    std::string ct = test::HexDecodeOrDie(test.ciphertext);
    std::string aad = test::HexDecodeOrDie(test.aad);
    auto res = createAead2(enc_key, test.iv_size, mac_key, test.tag_size,
                           test.hash_type);
    EXPECT_TRUE(res.ok()) << res.status();
    auto cipher = std::move(res.ValueOrDie());
    auto pt = cipher->Decrypt(ct, aad);
    EXPECT_TRUE(pt.ok()) << pt.status();
  }
}

TEST(EncryptThenAuthenticateTest, testEncryptDecrypt) {
  int encryption_key_size = 16;
  int iv_size = 12;
  int mac_key_size = 16;
  int tag_size = 16;
  auto res = createAead(encryption_key_size, iv_size, mac_key_size, tag_size,
                        HashType::SHA1);
  EXPECT_TRUE(res.ok()) << res.status();
  auto cipher = std::move(res.ValueOrDie());

  std::string message = "Some data to encrypt.";
  std::string aad = "Some data to authenticate.";
  auto ct = cipher->Encrypt(message, aad);
  EXPECT_TRUE(ct.ok()) << ct.status();
  EXPECT_EQ(ct.ValueOrDie().size(), message.size() + iv_size + tag_size);
  auto pt = cipher->Decrypt(ct.ValueOrDie(), aad);
  EXPECT_TRUE(pt.ok()) << pt.status();
  EXPECT_EQ(pt.ValueOrDie(), message);
}

TEST(EncryptThenAuthenticateTest, testEncryptDecrypt_randomMessage) {
  int encryption_key_size = 16;
  int iv_size = 12;
  int mac_key_size = 16;
  int tag_size = 16;
  auto res = createAead(encryption_key_size, iv_size, mac_key_size, tag_size,
                        HashType::SHA1);
  EXPECT_TRUE(res.ok()) << res.status();
  auto cipher = std::move(res.ValueOrDie());

  for (int i = 0; i < 256; i++) {
    std::string message = Random::GetRandomBytes(i);
    std::string aad = Random::GetRandomBytes(i);
    auto ct = cipher->Encrypt(message, aad);
    EXPECT_TRUE(ct.ok()) << ct.status();
    EXPECT_EQ(ct.ValueOrDie().size(), message.size() + iv_size + tag_size);
    auto pt = cipher->Decrypt(ct.ValueOrDie(), aad);
    EXPECT_TRUE(pt.ok()) << pt.status();
    EXPECT_EQ(pt.ValueOrDie(), message);
  }
}

TEST(AesCtrBoringSslTest, testMultipleEncrypt) {
  int encryption_key_size = 16;
  int iv_size = 12;
  int mac_key_size = 16;
  int tag_size = 16;
  auto res = createAead(encryption_key_size, iv_size, mac_key_size, tag_size,
                        HashType::SHA1);
  EXPECT_TRUE(res.ok()) << res.status();
  auto cipher = std::move(res.ValueOrDie());

  std::string message = Random::GetRandomBytes(20);
  std::string aad = Random::GetRandomBytes(20);
  auto ct1 = cipher->Encrypt(message, aad);
  auto ct2 = cipher->Encrypt(message, aad);
  EXPECT_NE(ct1.ValueOrDie(), ct2.ValueOrDie());
}

TEST(EncryptThenAuthenticateTest, testEncryptDecrypt_invalidTagSize) {
  int encryption_key_size = 16;
  int iv_size = 12;
  int mac_key_size = 16;
  int tag_size = 9;
  auto res = createAead(encryption_key_size, iv_size, mac_key_size, tag_size,
                        HashType::SHA1);
  EXPECT_FALSE(res.ok()) << res.status();
}

TEST(EncryptThenAuthenticateTest, testDecrypt_modifiedCiphertext) {
  int encryption_key_size = 16;
  int iv_size = 12;
  int mac_key_size = 16;
  int tag_size = 16;
  auto res = createAead(encryption_key_size, iv_size, mac_key_size, tag_size,
                        HashType::SHA1);
  EXPECT_TRUE(res.ok()) << res.status();
  auto cipher = std::move(res.ValueOrDie());

  std::string message = "Some data to encrypt.";
  std::string aad = "Some data to authenticate.";
  std::string ct = cipher->Encrypt(message, aad).ValueOrDie();
  EXPECT_TRUE(cipher->Decrypt(ct, aad).ok());
  // Modify the ciphertext
  for (size_t i = 0; i < ct.size() * 8; i++) {
    std::string modified_ct = ct;
    modified_ct[i / 8] ^= 1 << (i % 8);
    EXPECT_FALSE(cipher->Decrypt(modified_ct, aad).ok()) << i;
  }

  // Modify the additional data
  for (size_t i = 0; i < aad.size() * 8; i++) {
    std::string modified_aad = aad;
    modified_aad[i / 8] ^= 1 << (i % 8);
    auto decrypted = cipher->Decrypt(ct, modified_aad);
    EXPECT_FALSE(decrypted.ok()) << i << " pt:" << decrypted.ValueOrDie();
  }

  // Truncate the ciphertext
  for (size_t i = 0; i < ct.size(); i++) {
    std::string truncated_ct(ct, 0, i);
    EXPECT_FALSE(cipher->Decrypt(truncated_ct, aad).ok()) << i;
  }
}

TEST(EncryptThenAuthenticateTest, testParamsEmptyVersusNullStringView) {
  int encryption_key_size = 16;
  int iv_size = 12;
  int mac_key_size = 16;
  int tag_size = 16;
  auto cipher = std::move(
      createAead(encryption_key_size, iv_size, mac_key_size, tag_size,
                 HashType::SHA1).ValueOrDie());

  { // AAD null string_view.
    const std::string message = "Some data to encrypt.";
    const absl::string_view aad;
    const std::string ct = cipher->Encrypt(message, "").ValueOrDie();
    EXPECT_TRUE(cipher->Decrypt(ct, aad).ok());
  }
  { // Both message and AAD null string_view.
    const absl::string_view message;
    const absl::string_view aad;
    const std::string ct = cipher->Encrypt(message, "").ValueOrDie();
    EXPECT_TRUE(cipher->Decrypt(ct, aad).ok());
  }
}

}  // namespace
}  // namespace subtle
}  // namespace tink
}  // namespace crypto

int main(int ac, char* av[]) {
  testing::InitGoogleTest(&ac, av);
  return RUN_ALL_TESTS();
}
