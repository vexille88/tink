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

#include "tink/signature/public_key_sign_set_wrapper.h"

#include "tink/crypto_format.h"
#include "tink/primitive_set.h"
#include "tink/public_key_sign.h"
#include "tink/subtle/subtle_util_boringssl.h"
#include "tink/util/statusor.h"
#include "proto/tink.pb.h"

namespace crypto {
namespace tink {

using google::crypto::tink::OutputPrefixType;

namespace {

util::Status Validate(PrimitiveSet<PublicKeySign>* public_key_sign_set) {
  if (public_key_sign_set == nullptr) {
    return util::Status(util::error::INTERNAL,
                        "public_key_sign_set must be non-NULL");
  }
  if (public_key_sign_set->get_primary() == nullptr) {
    return util::Status(util::error::INVALID_ARGUMENT,
                        "public_key_sign_set has no primary");
  }
  return util::Status::OK;
}

}  // anonymous namespace

// static
util::StatusOr<std::unique_ptr<PublicKeySign>>
PublicKeySignSetWrapper::NewPublicKeySign(
    std::unique_ptr<PrimitiveSet<PublicKeySign>> public_key_sign_set) {
  util::Status status = Validate(public_key_sign_set.get());
  if (!status.ok()) return status;
  std::unique_ptr<PublicKeySign> public_key_sign(
      new PublicKeySignSetWrapper(std::move(public_key_sign_set)));
  return std::move(public_key_sign);
}

util::StatusOr<std::string> PublicKeySignSetWrapper::Sign(
    absl::string_view data) const {
  // BoringSSL expects a non-null pointer for data,
  // regardless of whether the size is 0.
  data = subtle::SubtleUtilBoringSSL::EnsureNonNull(data);

  auto primary = public_key_sign_set_->get_primary();
  std::string local_data;
  if (primary->get_output_prefix_type() == OutputPrefixType::LEGACY) {
    local_data = std::string(data);
    local_data.append(1, CryptoFormat::kLegacyStartByte);
    data = local_data;
  }
  auto sign_result = primary->get_primitive().Sign(data);
  if (!sign_result.ok()) return sign_result.status();
  const std::string& key_id = primary->get_identifier();
  return key_id + sign_result.ValueOrDie();
}

}  // namespace tink
}  // namespace crypto
