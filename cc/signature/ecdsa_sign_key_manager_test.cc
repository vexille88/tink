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

#include "tink/signature/ecdsa_sign_key_manager.h"

#include "tink/public_key_sign.h"
#include "tink/registry.h"
#include "tink/aead/aead_key_templates.h"
#include "tink/aead/aes_gcm_key_manager.h"
#include "tink/signature/signature_key_templates.h"
#include "tink/signature/ecdsa_verify_key_manager.h"
#include "tink/util/status.h"
#include "tink/util/statusor.h"
#include "tink/util/test_util.h"
#include "gtest/gtest.h"
#include "proto/common.pb.h"
#include "proto/ecdsa.pb.h"
#include "proto/aes_eax.pb.h"
#include "proto/tink.pb.h"

namespace crypto {
namespace tink {

using google::crypto::tink::AesEaxKey;
using google::crypto::tink::AesEaxKeyFormat;
using google::crypto::tink::EcdsaKeyFormat;
using google::crypto::tink::EcdsaPrivateKey;
using google::crypto::tink::EcdsaSignatureEncoding;
using google::crypto::tink::EllipticCurveType;
using google::crypto::tink::HashType;
using google::crypto::tink::KeyData;

namespace {

class EcdsaSignKeyManagerTest : public ::testing::Test {
 protected:
  std::string key_type_prefix_ = "type.googleapis.com/";
  std::string ecdsa_sign_key_type_ =
      "type.googleapis.com/google.crypto.tink.EcdsaPrivateKey";
};

// Checks whether given key is compatible with the given format.
void CheckNewKey(const EcdsaPrivateKey& ecdsa_key,
                 const EcdsaKeyFormat& key_format) {
  EcdsaSignKeyManager key_manager;
  EXPECT_EQ(0, ecdsa_key.version());
  EXPECT_TRUE(ecdsa_key.has_public_key());
  EXPECT_GT(ecdsa_key.key_value().length(), 0);
  EXPECT_EQ(0, ecdsa_key.public_key().version());
  EXPECT_GT(ecdsa_key.public_key().x().length(), 0);
  EXPECT_GT(ecdsa_key.public_key().y().length(), 0);
  EXPECT_EQ(ecdsa_key.public_key().params().SerializeAsString(),
            key_format.params().SerializeAsString());
  auto primitive_result = key_manager.GetPrimitive(ecdsa_key);
  EXPECT_TRUE(primitive_result.ok()) << primitive_result.status();
}

TEST_F(EcdsaSignKeyManagerTest, testBasic) {
  EcdsaSignKeyManager key_manager;

  EXPECT_EQ(0, key_manager.get_version());
  EXPECT_EQ("type.googleapis.com/google.crypto.tink.EcdsaPrivateKey",
            key_manager.get_key_type());
  EXPECT_TRUE(key_manager.DoesSupport(key_manager.get_key_type()));
}

TEST_F(EcdsaSignKeyManagerTest, testKeyDataErrors) {
  EcdsaSignKeyManager key_manager;

  {  // Bad key type.
    KeyData key_data;
    std::string bad_key_type =
        "type.googleapis.com/google.crypto.tink.SomeOtherKey";
    key_data.set_type_url(bad_key_type);
    auto result = key_manager.GetPrimitive(key_data);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(util::error::INVALID_ARGUMENT, result.status().error_code());
    EXPECT_PRED_FORMAT2(testing::IsSubstring, "not supported",
                        result.status().error_message());
    EXPECT_PRED_FORMAT2(testing::IsSubstring, bad_key_type,
                        result.status().error_message());
  }

  {  // Bad key value.
    KeyData key_data;
    key_data.set_type_url(ecdsa_sign_key_type_);
    key_data.set_value("some bad serialized proto");
    auto result = key_manager.GetPrimitive(key_data);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(util::error::INVALID_ARGUMENT, result.status().error_code());
    EXPECT_PRED_FORMAT2(testing::IsSubstring, "not parse",
                        result.status().error_message());
  }

  {  // Bad version.
    KeyData key_data;
    EcdsaPrivateKey key;
    key.set_version(1);
    key_data.set_type_url(ecdsa_sign_key_type_);
    key_data.set_value(key.SerializeAsString());
    auto result = key_manager.GetPrimitive(key_data);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(util::error::INVALID_ARGUMENT, result.status().error_code());
    EXPECT_PRED_FORMAT2(testing::IsSubstring, "version",
                        result.status().error_message());
  }
}

TEST_F(EcdsaSignKeyManagerTest, testKeyMessageErrors) {
  EcdsaSignKeyManager key_manager;

  {  // Bad protobuffer.
    AesEaxKey key;
    auto result = key_manager.GetPrimitive(key);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(util::error::INVALID_ARGUMENT, result.status().error_code());
    EXPECT_PRED_FORMAT2(testing::IsSubstring, "AesEaxKey",
                        result.status().error_message());
    EXPECT_PRED_FORMAT2(testing::IsSubstring, "not supported",
                        result.status().error_message());
  }

  {  // Bad elliptic curve.
    EcdsaPrivateKey key;
    auto public_key = key.mutable_public_key();
    public_key->mutable_params()->set_encoding(EcdsaSignatureEncoding::DER);
    public_key->mutable_params()->set_curve(EllipticCurveType::UNKNOWN_CURVE);
    auto result = key_manager.GetPrimitive(key);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(util::error::INVALID_ARGUMENT, result.status().error_code());
    EXPECT_PRED_FORMAT2(testing::IsSubstring, "Unsupported elliptic curve",
                        result.status().error_message());
  }

  {  // Bad hash type for NIST P256.
    EcdsaPrivateKey key;
    auto public_key = key.mutable_public_key();
    public_key->mutable_params()->set_encoding(EcdsaSignatureEncoding::DER);
    public_key->mutable_params()->set_curve(EllipticCurveType::NIST_P256);
    public_key->mutable_params()->set_hash_type(HashType::SHA512);
    auto result = key_manager.GetPrimitive(key);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(util::error::INVALID_ARGUMENT, result.status().error_code());
    EXPECT_PRED_FORMAT2(testing::IsSubstring, "Only SHA256",
                        result.status().error_message());
  }

  {  // Bad hash type for NIST P384.
    EcdsaPrivateKey key;
    auto public_key = key.mutable_public_key();
    public_key->mutable_params()->set_encoding(EcdsaSignatureEncoding::DER);
    public_key->mutable_params()->set_curve(EllipticCurveType::NIST_P384);
    public_key->mutable_params()->set_hash_type(HashType::SHA256);
    auto result = key_manager.GetPrimitive(key);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(util::error::INVALID_ARGUMENT, result.status().error_code());
    EXPECT_PRED_FORMAT2(testing::IsSubstring, "Only SHA512",
                        result.status().error_message());
  }

  {  // Bad hash type for NIST P521.
    EcdsaPrivateKey key;
    auto public_key = key.mutable_public_key();
    public_key->mutable_params()->set_encoding(EcdsaSignatureEncoding::DER);
    public_key->mutable_params()->set_curve(EllipticCurveType::NIST_P384);
    public_key->mutable_params()->set_hash_type(HashType::SHA256);
    auto result = key_manager.GetPrimitive(key);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(util::error::INVALID_ARGUMENT, result.status().error_code());
    EXPECT_PRED_FORMAT2(testing::IsSubstring, "Only SHA512",
                        result.status().error_message());
  }
}

TEST_F(EcdsaSignKeyManagerTest, testPrimitives) {
  EcdsaSignatureEncoding encodings[2] = {EcdsaSignatureEncoding::DER,
                                         EcdsaSignatureEncoding::IEEE_P1363};
  for (EcdsaSignatureEncoding encoding : encodings) {
    std::string message = "some message to sign";
    EcdsaSignKeyManager sign_key_manager;
    EcdsaPrivateKey key = test::GetEcdsaTestPrivateKey(
        EllipticCurveType::NIST_P256, HashType::SHA256, encoding);

    {  // Using Key proto.
      auto result = sign_key_manager.GetPrimitive(key);
      EXPECT_TRUE(result.ok()) << result.status();
      auto sign = std::move(result.ValueOrDie());
      auto signing_result = sign->Sign(message);
      EXPECT_TRUE(signing_result.ok()) << signing_result.status();
    }

    {  // Using KeyData proto.
      KeyData key_data;
      key_data.set_type_url(ecdsa_sign_key_type_);
      key_data.set_value(key.SerializeAsString());
      auto result = sign_key_manager.GetPrimitive(key_data);
      EXPECT_TRUE(result.ok()) << result.status();
      auto sign = std::move(result.ValueOrDie());
      auto signing_result = sign->Sign(message);
      EXPECT_TRUE(signing_result.ok()) << signing_result.status();
    }
  }
}

TEST_F(EcdsaSignKeyManagerTest, testNewKeyCreation) {
  EcdsaSignKeyManager key_manager;
  const KeyFactory& key_factory = key_manager.get_key_factory();

  { // Via NewKey(format_proto).
    EcdsaKeyFormat key_format;
    ASSERT_TRUE(key_format.ParseFromString(
        SignatureKeyTemplates::EcdsaP256().value()));
    auto result = key_factory.NewKey(key_format);
    EXPECT_TRUE(result.ok()) << result.status();
    auto key = std::move(result.ValueOrDie());
    ASSERT_EQ(ecdsa_sign_key_type_, key_type_prefix_ + key->GetTypeName());
    std::unique_ptr<EcdsaPrivateKey> ecdsa_key(
        reinterpret_cast<EcdsaPrivateKey*>(key.release()));
    CheckNewKey(*ecdsa_key, key_format);
  }

  { // Via NewKey(serialized_format_proto).
    EcdsaKeyFormat key_format;
    ASSERT_TRUE(key_format.ParseFromString(
        SignatureKeyTemplates::EcdsaP384().value()));
    auto result = key_factory.NewKey(key_format.SerializeAsString());
    EXPECT_TRUE(result.ok()) << result.status();
    auto key = std::move(result.ValueOrDie());
    ASSERT_EQ(ecdsa_sign_key_type_, key_type_prefix_ + key->GetTypeName());
    std::unique_ptr<EcdsaPrivateKey> ecdsa_key(
        reinterpret_cast<EcdsaPrivateKey*>(key.release()));
    CheckNewKey(*ecdsa_key, key_format);
  }

  { // Via NewKeyData(serialized_format_proto).
    EcdsaKeyFormat key_format;
    ASSERT_TRUE(key_format.ParseFromString(
        SignatureKeyTemplates::EcdsaP521().value()));
    auto result = key_factory.NewKeyData(key_format.SerializeAsString());
    EXPECT_TRUE(result.ok()) << result.status();
    auto key_data = std::move(result.ValueOrDie());
    EXPECT_EQ(ecdsa_sign_key_type_, key_data->type_url());
    EXPECT_EQ(KeyData::ASYMMETRIC_PRIVATE, key_data->key_material_type());
    EcdsaPrivateKey ecdsa_key;
    ASSERT_TRUE(ecdsa_key.ParseFromString(key_data->value()));
    CheckNewKey(ecdsa_key, key_format);
  }
}

TEST_F(EcdsaSignKeyManagerTest, testPublicKeyExtraction) {
  EcdsaSignKeyManager key_manager;
  auto private_key_factory = dynamic_cast<const PrivateKeyFactory*>(
      &(key_manager.get_key_factory()));
  ASSERT_NE(private_key_factory, nullptr);

  auto new_key_result = private_key_factory->NewKey(
      SignatureKeyTemplates::EcdsaP256().value());
  std::unique_ptr<EcdsaPrivateKey> new_key(
      reinterpret_cast<EcdsaPrivateKey*>(
          new_key_result.ValueOrDie().release()));
  auto public_key_data_result = private_key_factory->GetPublicKeyData(
      new_key->SerializeAsString());
  EXPECT_TRUE(public_key_data_result.ok()) << public_key_data_result.status();
  auto public_key_data = std::move(public_key_data_result.ValueOrDie());
  EXPECT_EQ(EcdsaVerifyKeyManager::kKeyType,
            public_key_data->type_url());
  EXPECT_EQ(KeyData::ASYMMETRIC_PUBLIC, public_key_data->key_material_type());
  EXPECT_EQ(new_key->public_key().SerializeAsString(),
            public_key_data->value());
}

TEST_F(EcdsaSignKeyManagerTest, testPublicKeyExtractionErrors) {
  EcdsaSignKeyManager key_manager;
  auto private_key_factory = dynamic_cast<const PrivateKeyFactory*>(
      &(key_manager.get_key_factory()));
  ASSERT_NE(private_key_factory, nullptr);

  AesGcmKeyManager aead_key_manager;
  auto aead_private_key_factory = dynamic_cast<const PrivateKeyFactory*>(
      &(aead_key_manager.get_key_factory()));
  ASSERT_EQ(nullptr, aead_private_key_factory);

  auto aead_key_result = aead_key_manager.get_key_factory().NewKey(
      AeadKeyTemplates::Aes128Gcm().value());
  ASSERT_TRUE(aead_key_result.ok()) << aead_key_result.status();
  auto aead_key = std::move(aead_key_result.ValueOrDie());
  auto public_key_data_result = private_key_factory->GetPublicKeyData(
      aead_key->SerializeAsString());
  EXPECT_FALSE(public_key_data_result.ok());
  EXPECT_EQ(util::error::INVALID_ARGUMENT,
            public_key_data_result.status().error_code());
}

TEST_F(EcdsaSignKeyManagerTest, testNewKeyErrors) {
  EcdsaSignKeyManager key_manager;
  const KeyFactory& key_factory = key_manager.get_key_factory();

  // Empty key format.
  EcdsaKeyFormat key_format;
  {
    auto result = key_factory.NewKey(key_format);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(util::error::INVALID_ARGUMENT, result.status().error_code());
    EXPECT_PRED_FORMAT2(testing::IsSubstring, "Missing params",
                        result.status().error_message());
  }

  // Wrong encoding.
  auto params = key_format.mutable_params();
  {
    auto result = key_factory.NewKey(key_format);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(util::error::INVALID_ARGUMENT, result.status().error_code());
    EXPECT_PRED_FORMAT2(testing::IsSubstring, "Unsupported signature encoding",
                        result.status().error_message());
  }
  // Wrong curve
  params->set_encoding(EcdsaSignatureEncoding::DER);
  {
    auto result = key_factory.NewKey(key_format);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(util::error::INVALID_ARGUMENT, result.status().error_code());
    EXPECT_PRED_FORMAT2(testing::IsSubstring, "Unsupported elliptic curve",
                        result.status().error_message());
  }

  // Wrong hash for the curve.
  params->set_curve(EllipticCurveType::NIST_P521);
  {
    auto result = key_factory.NewKey(key_format);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(util::error::INVALID_ARGUMENT, result.status().error_code());
    EXPECT_PRED_FORMAT2(testing::IsSubstring, "Only SHA512",
                        result.status().error_message());
  }

  // Bad serialized format.
  {
    auto result = key_factory.NewKey("some bad serialization");
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(util::error::INVALID_ARGUMENT, result.status().error_code());
    EXPECT_PRED_FORMAT2(testing::IsSubstring, "Could not parse",
                        result.status().error_message());
  }

  // Wrong format proto.
  {
    AesEaxKeyFormat wrong_key_format;
    auto result = key_factory.NewKey(wrong_key_format);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(util::error::INVALID_ARGUMENT, result.status().error_code());
    EXPECT_PRED_FORMAT2(testing::IsSubstring, "is not supported",
                        result.status().error_message());
  }
}

}  // namespace
}  // namespace tink
}  // namespace crypto


int main(int ac, char* av[]) {
  testing::InitGoogleTest(&ac, av);
  return RUN_ALL_TESTS();
}
