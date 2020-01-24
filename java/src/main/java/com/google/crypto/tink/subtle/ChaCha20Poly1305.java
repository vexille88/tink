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

package com.google.crypto.tink.subtle;

import java.security.InvalidKeyException;

/**
 * ChaCha20-Poly1305, as described in <a href="https://tools.ietf.org/html/rfc7539#section-2.8">RFC
 * 7539, section 2.8</a>.
 *
 * @since 1.1.0
 */
public final class ChaCha20Poly1305 extends SnufflePoly1305 {
  public ChaCha20Poly1305(final byte[] key) throws InvalidKeyException {
    super(key);
  }

  @Override
  Snuffle createSnuffleInstance(final byte[] key, int initialCounter) throws InvalidKeyException {
    return new ChaCha20(key, initialCounter);
  }
}
