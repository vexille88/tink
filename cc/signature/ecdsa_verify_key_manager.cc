// Copyright 2017 Google Inc.
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

#include "tink/signature/ecdsa_verify_key_manager.h"

#include "absl/strings/string_view.h"
#include "tink/public_key_verify.h"
#include "tink/key_manager.h"
#include "tink/subtle/ecdsa_verify_boringssl.h"
#include "tink/subtle/subtle_util_boringssl.h"
#include "tink/util/enums.h"
#include "tink/util/errors.h"
#include "tink/util/protobuf_helper.h"
#include "tink/util/status.h"
#include "tink/util/statusor.h"
#include "tink/util/validation.h"
#include "proto/ecdsa.pb.h"
#include "proto/tink.pb.h"

namespace crypto {
namespace tink {

using google::crypto::tink::EcdsaKeyFormat;
using google::crypto::tink::EcdsaParams;
using google::crypto::tink::EcdsaPublicKey;
using google::crypto::tink::EcdsaSignatureEncoding;
using google::crypto::tink::EllipticCurveType;
using google::crypto::tink::HashType;
using google::crypto::tink::KeyData;
using portable_proto::MessageLite;
using crypto::tink::util::Enums;
using crypto::tink::util::Status;
using crypto::tink::util::StatusOr;

class EcdsaPublicKeyFactory : public KeyFactory {
 public:
  EcdsaPublicKeyFactory() {}

  // Not implemented for public keys.
  crypto::tink::util::StatusOr<std::unique_ptr<portable_proto::MessageLite>>
  NewKey(const portable_proto::MessageLite& key_format) const override;

  // Not implemented for public keys.
  crypto::tink::util::StatusOr<std::unique_ptr<portable_proto::MessageLite>>
  NewKey(absl::string_view serialized_key_format) const override;

  // Not implemented for public keys.
  crypto::tink::util::StatusOr<std::unique_ptr<google::crypto::tink::KeyData>>
  NewKeyData(absl::string_view serialized_key_format) const override;
};

StatusOr<std::unique_ptr<MessageLite>> EcdsaPublicKeyFactory::NewKey(
    const portable_proto::MessageLite& key_format) const {
  return util::Status(util::error::UNIMPLEMENTED,
                      "Operation not supported for public keys, "
                      "please use the EcdsaSignKeyManager.");
}

StatusOr<std::unique_ptr<MessageLite>> EcdsaPublicKeyFactory::NewKey(
    absl::string_view serialized_key_format) const {
  return util::Status(util::error::UNIMPLEMENTED,
                      "Operation not supported for public keys, "
                      "please use the EcdsaSignKeyManager.");
}

StatusOr<std::unique_ptr<KeyData>> EcdsaPublicKeyFactory::NewKeyData(
    absl::string_view serialized_key_format) const {
  return util::Status(util::error::UNIMPLEMENTED,
                      "Operation not supported for public keys, "
                      "please use the EcdsaSignKeyManager.");
}

constexpr char EcdsaVerifyKeyManager::kKeyTypePrefix[];
constexpr char EcdsaVerifyKeyManager::kKeyType[];
constexpr uint32_t EcdsaVerifyKeyManager::kVersion;

EcdsaVerifyKeyManager::EcdsaVerifyKeyManager()
    : key_type_(kKeyType), key_factory_(new EcdsaPublicKeyFactory()) {
}

const std::string& EcdsaVerifyKeyManager::get_key_type() const {
  return key_type_;
}

const KeyFactory& EcdsaVerifyKeyManager::get_key_factory() const {
  return *key_factory_;
}

uint32_t EcdsaVerifyKeyManager::get_version() const {
  return kVersion;
}

StatusOr<std::unique_ptr<PublicKeyVerify>>
EcdsaVerifyKeyManager::GetPrimitive(const KeyData& key_data) const {
  if (DoesSupport(key_data.type_url())) {
    EcdsaPublicKey ecdsa_public_key;
    if (!ecdsa_public_key.ParseFromString(key_data.value())) {
      return ToStatusF(util::error::INVALID_ARGUMENT,
                       "Could not parse key_data.value as key type '%s'.",
                       key_data.type_url().c_str());
    }
    return GetPrimitiveImpl(ecdsa_public_key);
  } else {
    return ToStatusF(util::error::INVALID_ARGUMENT,
                     "Key type '%s' is not supported by this manager.",
                     key_data.type_url().c_str());
  }
}

StatusOr<std::unique_ptr<PublicKeyVerify>>
EcdsaVerifyKeyManager::GetPrimitive(const MessageLite& key) const {
  std::string key_type = std::string(kKeyTypePrefix) + key.GetTypeName();
  if (DoesSupport(key_type)) {
    const EcdsaPublicKey& ecdsa_public_key =
        reinterpret_cast<const EcdsaPublicKey&>(key);
    return GetPrimitiveImpl(ecdsa_public_key);
  } else {
    return ToStatusF(util::error::INVALID_ARGUMENT,
                     "Key type '%s' is not supported by this manager.",
                     key_type.c_str());
  }
}

StatusOr<std::unique_ptr<PublicKeyVerify>>
EcdsaVerifyKeyManager::GetPrimitiveImpl(
    const EcdsaPublicKey& ecdsa_public_key) const {
  Status status = Validate(ecdsa_public_key);
  if (!status.ok()) return status;
  subtle::SubtleUtilBoringSSL::EcKey ec_key;
  ec_key.curve = Enums::ProtoToSubtle(ecdsa_public_key.params().curve());
  ec_key.pub_x = ecdsa_public_key.x();
  ec_key.pub_y = ecdsa_public_key.y();
  auto ecdsa_result = subtle::EcdsaVerifyBoringSsl::New(
      ec_key, Enums::ProtoToSubtle(ecdsa_public_key.params().hash_type()),
      Enums::ProtoToSubtle(ecdsa_public_key.params().encoding()));
  if (!ecdsa_result.ok()) return ecdsa_result.status();
  std::unique_ptr<PublicKeyVerify> ecdsa(ecdsa_result.ValueOrDie().release());
  return std::move(ecdsa);
}

// static
Status EcdsaVerifyKeyManager::Validate(const EcdsaParams& params) {
  switch (params.encoding()) {
    case EcdsaSignatureEncoding::DER:  // fall through
    case EcdsaSignatureEncoding::IEEE_P1363:
      break;
    default:
      return ToStatusF(util::error::INVALID_ARGUMENT,
                       "Unsupported signature encoding: %d", params.encoding());
  }
  switch (params.curve()) {
    case EllipticCurveType::NIST_P256:
      // Using SHA512 for curve P256 is fine. However, only the 256
      // leftmost bits of the hash is used in signature computation.
      // Therefore, we don't allow it here to prevent security illusion.
      if (params.hash_type() != HashType::SHA256) {
        return Status(util::error::INVALID_ARGUMENT,
                      "Only SHA256 is supported for NIST P256.");
      }
      break;
    case EllipticCurveType::NIST_P384:
    case EllipticCurveType::NIST_P521:
      if (params.hash_type() != HashType::SHA512) {
        return Status(util::error::INVALID_ARGUMENT,
                      "Only SHA512 is supported for this curve.");
      }
      break;
    default:
      return Status(util::error::INVALID_ARGUMENT,
                    "Unsupported elliptic curve");
  }
  return Status::OK;
}

// static
Status EcdsaVerifyKeyManager::Validate(
    const EcdsaPublicKey& key) {
  Status status = ValidateVersion(key.version(), kVersion);
  if (!status.ok()) return status;
  return Validate(key.params());
}

// static
Status EcdsaVerifyKeyManager::Validate(
    const EcdsaKeyFormat& key_format) {
  if (!key_format.has_params()) {
    return Status(util::error::INVALID_ARGUMENT, "Missing params.");
  }
  return Validate(key_format.params());
}

}  // namespace tink
}  // namespace crypto
