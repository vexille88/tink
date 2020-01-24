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

#ifndef TINK_SIGNATURE_PUBLIC_KEY_SIGN_SET_WRAPPER_H_
#define TINK_SIGNATURE_PUBLIC_KEY_SIGN_SET_WRAPPER_H_

#include "absl/strings/string_view.h"
#include "tink/public_key_sign.h"
#include "tink/primitive_set.h"
#include "tink/util/statusor.h"
#include "proto/tink.pb.h"

namespace crypto {
namespace tink {

// Wraps a set of PublicKeySign-instances that correspond to a keyset,
// and combines them into a single PublicKeySign-primitive,
// that for the actual verification uses the instance that maches the
// signature prefix.
class PublicKeySignSetWrapper : public PublicKeySign {
 public:
  // Returns an PublicKeySign-primitive that uses the primary
  // PublicKeySign-instance provided in 'public_key_sign_set',
  // which must be non-NULL (and must contain a primary instance).
  static crypto::tink::util::StatusOr<std::unique_ptr<PublicKeySign>>
      NewPublicKeySign(
          std::unique_ptr<PrimitiveSet<PublicKeySign>> public_key_sign_set);

  crypto::tink::util::StatusOr<std::string> Sign(absl::string_view data)
      const override;

  virtual ~PublicKeySignSetWrapper() {}

 private:
  std::unique_ptr<PrimitiveSet<PublicKeySign>> public_key_sign_set_;

  PublicKeySignSetWrapper(
      std::unique_ptr<PrimitiveSet<PublicKeySign>> public_key_sign_set)
      : public_key_sign_set_(std::move(public_key_sign_set)) {}
};

}  // namespace tink
}  // namespace crypto

#endif  // TINK_SIGNATURE_PUBLIC_KEY_SIGN_SET_WRAPPER_H_
