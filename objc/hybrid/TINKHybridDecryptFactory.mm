/**
 * Copyright 2017 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 **************************************************************************
 */

#import "objc/TINKHybridDecryptFactory.h"

#import <Foundation/Foundation.h>

#import "objc/TINKHybridDecrypt.h"
#import "objc/TINKKeysetHandle.h"
#import "objc/core/TINKKeysetHandle_Internal.h"
#import "objc/hybrid/TINKHybridDecryptInternal.h"
#import "objc/util/TINKErrors.h"

#include "tink/hybrid/hybrid_decrypt_factory.h"
#include "tink/keyset_handle.h"
#include "tink/util/status.h"

@implementation TINKHybridDecryptFactory

+ (id<TINKHybridDecrypt>)primitiveWithKeysetHandle:(TINKKeysetHandle *)keysetHandle
                                             error:(NSError **)error {
  crypto::tink::KeysetHandle *handle = [keysetHandle ccKeysetHandle];

  auto st = crypto::tink::HybridDecryptFactory::GetPrimitive(*handle);
  if (!st.ok()) {
    if (error) {
      *error = TINKStatusToError(st.status());
    }
    return nil;
  }
  id<TINKHybridDecrypt> hybrid =
      [[TINKHybridDecryptInternal alloc] initWithCCHybridDecrypt:std::move(st.ValueOrDie())];
  if (!hybrid) {
    if (error) {
      *error = TINKStatusToError(crypto::tink::util::Status(
          crypto::tink::util::error::RESOURCE_EXHAUSTED, "Cannot initialize TINKHybridDecrypt"));
    }
    return nil;
  }

  return hybrid;
}

@end
