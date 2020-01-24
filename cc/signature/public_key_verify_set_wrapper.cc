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

#include "tink/signature/public_key_verify_set_wrapper.h"

#include "tink/crypto_format.h"
#include "tink/primitive_set.h"
#include "tink/public_key_verify.h"
#include "tink/subtle/subtle_util_boringssl.h"
#include "tink/util/status.h"
#include "tink/util/statusor.h"
#include "proto/tink.pb.h"

namespace crypto {
namespace tink {

using google::crypto::tink::OutputPrefixType;

namespace {

util::Status Validate(PrimitiveSet<PublicKeyVerify>* public_key_verify_set) {
  if (public_key_verify_set == nullptr) {
    return util::Status(util::error::INTERNAL,
                        "public_key_verify_set must be non-NULL");
  }
  if (public_key_verify_set->get_primary() == nullptr) {
    return util::Status(util::error::INVALID_ARGUMENT,
                        "public_key_verify_set has no primary");
  }
  return util::Status::OK;
}

}  // anonymous namespace

// static
util::StatusOr<std::unique_ptr<PublicKeyVerify>>
PublicKeyVerifySetWrapper::NewPublicKeyVerify(
    std::unique_ptr<PrimitiveSet<PublicKeyVerify>> public_key_verify_set) {
  util::Status status = Validate(public_key_verify_set.get());
  if (!status.ok()) return status;
  std::unique_ptr<PublicKeyVerify> public_key_verify(
      new PublicKeyVerifySetWrapper(std::move(public_key_verify_set)));
  return std::move(public_key_verify);
}

util::Status PublicKeyVerifySetWrapper::Verify(
    absl::string_view signature,
    absl::string_view data) const {
  // BoringSSL expects a non-null pointer for data,
  // regardless of whether the size is 0.
  data = subtle::SubtleUtilBoringSSL::EnsureNonNull(data);
  signature = subtle::SubtleUtilBoringSSL::EnsureNonNull(signature);

  if (signature.length() <= CryptoFormat::kNonRawPrefixSize) {
    // This also rejects raw signatures with size of 4 bytes or fewer.
    // We're not aware of any schemes that output signatures that small.
    return util::Status(util::error::INVALID_ARGUMENT, "Signature too short.");
  }
  const std::string& key_id = std::string(
      signature.substr(0, CryptoFormat::kNonRawPrefixSize));
  auto primitives_result = public_key_verify_set_->get_primitives(key_id);
  if (primitives_result.ok()) {
    absl::string_view raw_signature =
        signature.substr(CryptoFormat::kNonRawPrefixSize);
    std::string local_data;
    for (auto& entry : *(primitives_result.ValueOrDie())) {
      if (entry->get_output_prefix_type() == OutputPrefixType::LEGACY) {
        local_data = std::string(data);
        local_data.append(1, CryptoFormat::kLegacyStartByte);
        data = local_data;
      }
      auto& public_key_verify = entry->get_primitive();
      auto verify_result =
          public_key_verify.Verify(raw_signature, data);
      if (verify_result.ok()) {
        return util::Status::OK;
      } else {
        // LOG that a matching key didn't verify the signature.
      }
    }
  }

  // No matching key succeeded with verification, try all RAW keys.
  auto raw_primitives_result = public_key_verify_set_->get_raw_primitives();
  if (raw_primitives_result.ok()) {
    for (auto& public_key_verify_entry :
             *(raw_primitives_result.ValueOrDie())) {
      auto& public_key_verify = public_key_verify_entry->get_primitive();
      auto verify_result = public_key_verify.Verify(signature, data);
      if (verify_result.ok()) {
        return util::Status::OK;
      }
    }
  }
  return util::Status(util::error::INVALID_ARGUMENT, "Invalid signature.");
}

}  // namespace tink
}  // namespace crypto
