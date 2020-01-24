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

#include "tink/subtle/aes_eax_boringssl.h"

#include <string>
#include <vector>

#include "absl/strings/str_cat.h"
#include "tink/subtle/wycheproof_util.h"
#include "tink/util/status.h"
#include "tink/util/statusor.h"
#include "tink/util/test_util.h"
#include "gtest/gtest.h"
#include "openssl/err.h"

namespace crypto {
namespace tink {
namespace subtle {
namespace {

TEST(AesEaxBoringSslTest, testBasic) {
  std::string key(test::HexDecodeOrDie("000102030405060708090a0b0c0d0e0f"));
  size_t nonce_size = 12;
  auto res = AesEaxBoringSsl::New(key, nonce_size);
  EXPECT_TRUE(res.ok()) << res.status();
  auto cipher = std::move(res.ValueOrDie());
  std::string message = "Some data to encrypt.";
  std::string aad = "Some data to authenticate.";
  auto ct = cipher->Encrypt(message, aad);
  EXPECT_TRUE(ct.ok()) << ct.status();
  EXPECT_EQ(ct.ValueOrDie().size(), message.size() + nonce_size + 16);
  auto pt = cipher->Decrypt(ct.ValueOrDie(), aad);
  EXPECT_TRUE(pt.ok()) << pt.status();
  EXPECT_EQ(pt.ValueOrDie(), message);
}

TEST(AesEaxBoringSslTest, testMessageSize) {
  std::string key(test::HexDecodeOrDie("000102030405060708090a0b0c0d0e0f"));
  size_t nonce_size = 12;
  auto res = AesEaxBoringSsl::New(key, nonce_size);
  EXPECT_TRUE(res.ok()) << res.status();
  auto cipher = std::move(res.ValueOrDie());
  for (size_t size = 0; size < 260; size++) {
    std::string message(size, 'x');
    std::string aad = "";
    auto ct = cipher->Encrypt(message, aad);
    EXPECT_TRUE(ct.ok()) << ct.status();
    EXPECT_EQ(ct.ValueOrDie().size(), message.size() + nonce_size + 16);
    auto pt = cipher->Decrypt(ct.ValueOrDie(), aad);
    EXPECT_TRUE(pt.ok()) << pt.status();
    EXPECT_EQ(pt.ValueOrDie(), message);
  }
}

TEST(AesEaxBoringSslTest, testAadSize) {
  std::string key(test::HexDecodeOrDie("000102030405060708090a0b0c0d0e0f"));
  size_t nonce_size = 12;
  auto res = AesEaxBoringSsl::New(key, nonce_size);
  EXPECT_TRUE(res.ok()) << res.status();
  auto cipher = std::move(res.ValueOrDie());
  for (size_t size = 0; size < 260; size++) {
    std::string message("Some message");
    std::string aad(size, 'x');
    auto ct = cipher->Encrypt(message, aad);
    EXPECT_TRUE(ct.ok()) << ct.status();
    EXPECT_EQ(ct.ValueOrDie().size(), message.size() + nonce_size + 16);
    auto pt = cipher->Decrypt(ct.ValueOrDie(), aad);
    EXPECT_TRUE(pt.ok()) << pt.status();
    EXPECT_EQ(pt.ValueOrDie(), message);
  }
}

TEST(AesEaxBoringSslTest, testLongNonce) {
  std::string key(test::HexDecodeOrDie("000102030405060708090a0b0c0d0e0f"));
  size_t nonce_size = 16;
  auto res = AesEaxBoringSsl::New(key, nonce_size);
  EXPECT_TRUE(res.ok()) << res.status();
  auto cipher = std::move(res.ValueOrDie());
  std::string message = "Some data to encrypt.";
  std::string aad = "Some data to authenticate.";
  auto ct = cipher->Encrypt(message, aad);
  EXPECT_TRUE(ct.ok()) << ct.status();
  EXPECT_EQ(ct.ValueOrDie().size(), message.size() + nonce_size + 16);
  auto pt = cipher->Decrypt(ct.ValueOrDie(), aad);
  EXPECT_TRUE(pt.ok()) << pt.status();
  EXPECT_EQ(pt.ValueOrDie(), message);
}

TEST(AesEaxBoringSslTest, testModification) {
  size_t nonce_size = 12;
  std::string key(test::HexDecodeOrDie("000102030405060708090a0b0c0d0e0f"));
  auto cipher = std::move(AesEaxBoringSsl::New(key, nonce_size).ValueOrDie());
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

TEST(AesEaxBoringSslTest, testInvalidKeySizes) {
  size_t nonce_size = 12;
  for (int keysize = 0; keysize < 65; keysize++) {
    if (keysize == 16 || keysize == 32) {
      continue;
    }
    std::string key(keysize, 'x');
    auto cipher = AesEaxBoringSsl::New(key, nonce_size);
    EXPECT_FALSE(cipher.ok());
  }
  absl::string_view null_string_view;
  auto nokeycipher = AesEaxBoringSsl::New(null_string_view, nonce_size);
  EXPECT_FALSE(nokeycipher.ok());
}

TEST(AesEaxBoringSslTest, testEmpty) {
  size_t nonce_size = 12;
  std::string key(test::HexDecodeOrDie("bedcfb5a011ebc84600fcb296c15af0d"));
  std::string nonce(test::HexDecodeOrDie("438a547a94ea88dce46c6c85"));
  // Expected tag is an empty std::string with an empty tag is encrypted with
  // the nonce above;
  std::string tag(test::HexDecodeOrDie("9607977cd7556b1dfedf0c73a35a5197"));
  std::string ciphertext = nonce + tag;
  auto res = AesEaxBoringSsl::New(key, nonce_size);
  EXPECT_TRUE(res.ok()) << res.status();
  auto cipher = std::move(res.ValueOrDie());

  // Test decryption of the arguments above.
  std::string empty_string("");
  absl::string_view empty_string_view("");
  absl::string_view null_string_view;

  auto pt = cipher->Decrypt(ciphertext, empty_string);
  EXPECT_TRUE(pt.ok());
  EXPECT_EQ(0, pt.ValueOrDie().size());

  pt = cipher->Decrypt(ciphertext, empty_string_view);
  EXPECT_TRUE(pt.ok());
  EXPECT_EQ(0, pt.ValueOrDie().size());

  pt = cipher->Decrypt(ciphertext, null_string_view);
  EXPECT_TRUE(pt.ok());
  EXPECT_EQ(0, pt.ValueOrDie().size());

  // Test encryption.
  auto ct = cipher->Encrypt(empty_string, empty_string);
  EXPECT_TRUE(ct.ok());
  pt = cipher->Decrypt(ct.ValueOrDie(), empty_string);
  EXPECT_TRUE(pt.ok());
  EXPECT_EQ(0, pt.ValueOrDie().size());

  ct = cipher->Encrypt(empty_string_view, empty_string_view);
  EXPECT_TRUE(ct.ok());
  pt = cipher->Decrypt(ct.ValueOrDie(), empty_string);
  EXPECT_TRUE(pt.ok());
  EXPECT_EQ(0, pt.ValueOrDie().size());

  ct = cipher->Encrypt(empty_string_view, empty_string_view);
  EXPECT_TRUE(ct.ok());
  pt = cipher->Decrypt(ct.ValueOrDie(), empty_string);
  EXPECT_TRUE(pt.ok());
  EXPECT_EQ(0, pt.ValueOrDie().size());

  ct = cipher->Encrypt(null_string_view, null_string_view);
  EXPECT_TRUE(ct.ok());
  pt = cipher->Decrypt(ct.ValueOrDie(), empty_string);
  EXPECT_TRUE(pt.ok());
  EXPECT_EQ(0, pt.ValueOrDie().size());
}

static std::string GetError() {
  auto err = ERR_peek_last_error();
  // Sometimes there is no error message on the stack.
  if (err == 0) {
    return "";
  }
  std::string lib(ERR_lib_error_string(err));
  std::string func(ERR_func_error_string(err));
  std::string reason(ERR_reason_error_string(err));
  return lib + ":" + func + ":" + reason;
}

// Test with test vectors from project Wycheproof.
// AesEaxBoringSsl does not allow to pass in IVs. Therefore this test
// can only test decryption.
// Currently AesEaxBoringSsl is restricted to encryption with 12 byte
// IVs and 16 byte tags. Therefore it is necessary to skip tests with
// other parameter sizes.
bool WycheproofTest(const rapidjson::Document &root) {
  int errors = 0;
  for (const rapidjson::Value& test_group : root["testGroups"].GetArray()) {
    const size_t iv_size = test_group["ivSize"].GetInt();
    const size_t key_size = test_group["keySize"].GetInt();
    const size_t tag_size = test_group["tagSize"].GetInt();
    if (key_size != 128 && key_size != 256) {
      // Not supported
      continue;
    }
    if (iv_size != 128 && iv_size != 96) {
      // Not supported
      continue;
    }
    if (tag_size != 128) {
      // Not supported
      continue;
    }
    for (const rapidjson::Value& test : test_group["tests"].GetArray()) {
      std::string comment = test["comment"].GetString();
      std::string key = WycheproofUtil::GetBytes(test["key"]);
      std::string iv = WycheproofUtil::GetBytes(test["iv"]);
      std::string msg = WycheproofUtil::GetBytes(test["msg"]);
      std::string ct = WycheproofUtil::GetBytes(test["ct"]);
      std::string aad = WycheproofUtil::GetBytes(test["aad"]);
      std::string tag = WycheproofUtil::GetBytes(test["tag"]);
      std::string id = absl::StrCat(test["tcId"].GetInt());
      std::string expected = test["result"].GetString();
      auto cipher =
         std::move(AesEaxBoringSsl::New(key, iv_size / 8).ValueOrDie());
      auto result = cipher->Decrypt(iv + ct + tag, aad);
      bool success = result.ok();
      if (success) {
        std::string decrypted = result.ValueOrDie();
        if (expected == "invalid") {
          ADD_FAILURE() << "decrypted invalid ciphertext:" << id;
          errors++;
        } else if (msg != decrypted) {
          ADD_FAILURE() << "Incorrect decryption:" << id;
          errors++;
        }
      } else {
        if (expected == "valid" || expected == "acceptable") {
          ADD_FAILURE()
              << "Could not decrypt test with tcId:" << id
              << " iv_size:" << iv_size
              << " tag_size:" << tag_size
              << " key_size:" << key_size
              << " error:" << GetError();
          errors++;
        }
      }
    }
  }
  return errors == 0;
}

TEST(AesEaxBoringSslTest, TestVectors) {
  std::unique_ptr<rapidjson::Document> root =
      WycheproofUtil::ReadTestVectors("aes_eax_test.json");
  ASSERT_TRUE(WycheproofTest(*root));
}

}  // namespace
}  // namespace subtle
}  // namespace tink
}  // namespace crypto

int main(int ac, char* av[]) {
  testing::InitGoogleTest(&ac, av);
  return RUN_ALL_TESTS();
}
