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

#ifndef TINK_MAC_MAC_SET_WRAPPER_H_
#define TINK_MAC_MAC_SET_WRAPPER_H_

#include "absl/strings/string_view.h"
#include "tink/mac.h"
#include "tink/primitive_set.h"
#include "tink/util/status.h"
#include "tink/util/statusor.h"
#include "proto/tink.pb.h"

namespace crypto {
namespace tink {

// Wraps a set of Mac-instances that correspond to a keyset,
// and combines them into a single Mac-primitive, that uses the provided
// instances, depending on the context:
//   * Mac::ComputeMac(...) uses the primary instance from the set
//   * Mac::VerifyMac(...) uses the instance that matches the MAC prefix.
class MacSetWrapper : public Mac {
 public:
  // Returns an Mac-primitive that uses Mac-instances provided in 'mac_set',
  // which must be non-NULL and must contain a primary instance.
  static crypto::tink::util::StatusOr<std::unique_ptr<Mac>> NewMac(
      std::unique_ptr<PrimitiveSet<Mac>> mac_set);

  crypto::tink::util::StatusOr<std::string> ComputeMac(
      absl::string_view data) const override;

  crypto::tink::util::Status VerifyMac(
      absl::string_view mac_value,
      absl::string_view data) const override;

  virtual ~MacSetWrapper() {}

 private:
  std::unique_ptr<PrimitiveSet<Mac>> mac_set_;

  MacSetWrapper(std::unique_ptr<PrimitiveSet<Mac>> mac_set)
      : mac_set_(std::move(mac_set)) {}
};

}  // namespace tink
}  // namespace crypto

#endif  // TINK_MAC_MAC_SET_WRAPPER_H_
