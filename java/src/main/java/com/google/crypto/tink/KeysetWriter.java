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

import com.google.crypto.tink.proto.EncryptedKeyset;
import com.google.crypto.tink.proto.Keyset;
import java.io.IOException;

/**
 * A KeysetWriter knows how to write a {@link Keyset} or an {@link EncryptedKeyset} to some storage
 * system.
 *
 * @since 1.0.0
 */
public interface KeysetWriter {
  /** Tries to write a {@link Keyset} to some storage system. */
  void write(Keyset keyset) throws IOException;

  /** Tries to write an {@link EncryptedKeyset} to some storage system. */
  void write(EncryptedKeyset keyset) throws IOException;
}
