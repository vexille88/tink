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

package com.google.crypto.tink.tinkey;

import com.google.crypto.tink.KeysetHandle;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.security.GeneralSecurityException;

/**
 * Changes format, encrypts or decrypts a keyset.
 */
public class ConvertKeysetCommand extends ConvertKeysetOptions implements Command {
  @Override
  public void run() throws Exception {
    validate();
    convert(outputStream, outFormat, inputStream, inFormat, masterKeyUri, credentialPath,
        newMasterKeyUri, newCredentialPath);
  }

  /**
   * Changes format, encrypts or decrypts a keyset.
   *
   * <p>The keyset is read from {@code inputStream}. Its format can be either <code>json</code>
   * or <code>binary</code>, and is specified by {@code inFormat}. The new key is generated
   * from template {@code keyTemplate}. If the input keyset is encrypted, use
   * {@code masterKeyUri} and {@code credentialPath} to decrypt. The output keyset
   * is written to {@code outputStream} in {@code outFormat}, and encrypted if the
   * input keyset is encrypted.
   *
   * @throws GeneralSecurityException if cannot encrypt/decrypt the keyset
   * @throws IOException if cannot read/write the keyset
   */
  public static void convert(OutputStream outputStream, String outFormat,
      InputStream inputStream, String inFormat,
      String masterKeyUri, String credentialPath,
      String newMasterKeyUri, String newCredentialPath)
      throws GeneralSecurityException, IOException {
    KeysetHandle handle = TinkeyUtil.getKeysetHandle(inputStream, inFormat, masterKeyUri,
        credentialPath);
    TinkeyUtil.writeKeyset(handle, outputStream, outFormat, newMasterKeyUri, newCredentialPath);
  }
}
