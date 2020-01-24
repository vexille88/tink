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

#include <algorithm>
#include <vector>

#ifndef TINK_MAC_HMAC_KEY_MANAGER_H_
#define TINK_MAC_HMAC_KEY_MANAGER_H_

#include "absl/strings/string_view.h"
#include "tink/mac.h"
#include "tink/key_manager.h"
#include "tink/util/errors.h"
#include "tink/util/protobuf_helper.h"
#include "tink/util/status.h"
#include "tink/util/statusor.h"
#include "proto/hmac.pb.h"
#include "proto/tink.pb.h"

namespace crypto {
namespace tink {

class HmacKeyManager : public KeyManager<Mac> {
 public:
  static constexpr char kKeyType[] =
      "type.googleapis.com/google.crypto.tink.HmacKey";
  static constexpr uint32_t kVersion = 0;

  HmacKeyManager();

  // Constructs an instance of HMAC-Mac for the given 'key_data',
  // which must contain HmacKey-proto.
  crypto::tink::util::StatusOr<std::unique_ptr<Mac>> GetPrimitive(
      const google::crypto::tink::KeyData& key_data) const override;

  // Constructs an instance of HMAC-Mac for the given 'key',
  // which must be HmacKey-proto.
  crypto::tink::util::StatusOr<std::unique_ptr<Mac>>
  GetPrimitive(const portable_proto::MessageLite& key) const override;

  // Returns the type_url identifying the key type handled by this manager.
  const std::string& get_key_type() const override;

  // Returns the version of this key manager.
  uint32_t get_version() const override;

  // Returns a factory that generates keys of the key type
  // handled by this manager.
  const KeyFactory& get_key_factory() const override;

  virtual ~HmacKeyManager() {}

 private:
  friend class HmacKeyFactory;

  static constexpr char kKeyTypePrefix[] = "type.googleapis.com/";
  static constexpr char kKeyFormatUrl[] =
      "type.googleapis.com/google.crypto.tink.HmacKeyFormat";

  std::string key_type_;
  std::unique_ptr<KeyFactory> key_factory_;

  // Constructs an instance of HMAC-Mac for the given 'key'.
  crypto::tink::util::StatusOr<std::unique_ptr<Mac>>
  GetPrimitiveImpl(const google::crypto::tink::HmacKey& key) const;

  static crypto::tink::util::Status Validate(
      const google::crypto::tink::HmacParams& params);
  static crypto::tink::util::Status Validate(
      const google::crypto::tink::HmacKey& key);
  static crypto::tink::util::Status Validate(
      const google::crypto::tink::HmacKeyFormat& key_format);
};

}  // namespace tink
}  // namespace crypto

#endif  // TINK_MAC_HMAC_KEY_MANAGER_H_
