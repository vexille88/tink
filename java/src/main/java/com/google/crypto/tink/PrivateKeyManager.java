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

package com.google.crypto.tink;

import com.google.crypto.tink.proto.KeyData;
import com.google.protobuf.ByteString;
import java.security.GeneralSecurityException;

/**
 * A {@link KeyManager} that understands asymmetric private key types.
 *
 * @since 1.0.0
 */
public interface PrivateKeyManager<P> extends KeyManager<P> {
  /**
   * Extracts the public key data from the private key data.
   *
   * @return the {@link KeyData} containing the public keys
   * @throws GeneralSecurityException if the specified format is wrong or not supported
   */
  KeyData getPublicKeyData(ByteString serializedKey) throws GeneralSecurityException;
}
