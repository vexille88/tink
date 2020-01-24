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

#ifndef TINK_SIGNATURE_PUBLIC_KEY_VERIFY_FACTORY_H_
#define TINK_SIGNATURE_PUBLIC_KEY_VERIFY_FACTORY_H_

#include "tink/public_key_verify.h"
#include "tink/key_manager.h"
#include "tink/keyset_handle.h"
#include "tink/util/statusor.h"

namespace crypto {
namespace tink {

///////////////////////////////////////////////////////////////////////////////
// PublicKeyVerifyFactory allows for obtaining an PublicKeyVerify primitive
// from a KeysetHandle.
//
// PublicKeyVerifyFactory gets primitives from the Registry, which can
// be initialized via a convenience method from SignatureConfig-class.
// Here is an example how one can obtain and use a PublicKeyVerify primitive:
//
//   auto status = SignatureConfig::Register();
//   if (!status.ok()) { /* fail with error */ }
//   KeysetHandle keyset_handle = ...;
//   std::unique_ptr<PublicKeyVerify> public_key_verify = std::move(
//           PublicKeyVerifyFactory::GetPrimitive(keyset_handle).ValueOrDie());
//   std::string data = ...;
//   std::string signature = ...;
//   status = public_key_verify.Verify(signature, data);
//   if (!status.ok()) {
//     // Signature was not correct.
//     // ...
//   }
//
class PublicKeyVerifyFactory {
 public:
  // Returns a PublicKeyVerify-primitive that uses key material from the keyset
  // specified via 'keyset_handle'.
  static crypto::tink::util::StatusOr<std::unique_ptr<PublicKeyVerify>>
      GetPrimitive(const KeysetHandle& keyset_handle);

  // Returns a PublicKeyVerify-primitive that uses key material from the keyset
  // specified via 'keyset_handle' and is instantiated by the given
  // 'custom_key_manager' (instead of the key manager from the Registry).
  static crypto::tink::util::StatusOr<std::unique_ptr<PublicKeyVerify>>
      GetPrimitive(const KeysetHandle& keyset_handle,
                   const KeyManager<PublicKeyVerify>* custom_key_manager);

 private:
  PublicKeyVerifyFactory() {}
};

}  // namespace tink
}  // namespace crypto

#endif  // TINK_SIGNATURE_PUBLIC_KEY_VERIFY_FACTORY_H_
