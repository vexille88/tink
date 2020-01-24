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

import com.google.crypto.tink.Mac;
import com.google.crypto.tink.annotations.Alpha;
import java.security.GeneralSecurityException;
import java.security.InvalidAlgorithmParameterException;
import java.util.Arrays;
import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.spec.SecretKeySpec;

/**
 * An implementation of CMAC following <a href="https://tools.ietf.org/html/rfc4493">RFC 4493</a>.
 *
 * <h3>Warning</h3>
 *
 * <p>Do not use this API or any other APIs including fields and methods marked with the @Alpha
 * annotation. They can be modified in any way, or even removed, at any time. They are in the
 * package, but not for official, production release, but only for testing.
 */
@Alpha
public final class AesCmac implements Mac {
  static final int MIN_TAG_SIZE_IN_BYTES = 10;

  private final SecretKey keySpec;
  private final int tagSizeInBytes;
  private byte[] subKey1;
  private byte[] subKey2;

  private static Cipher instance() throws GeneralSecurityException {
    return EngineFactory.CIPHER.getInstance("AES/ECB/NoPadding");
  }

  public AesCmac(final byte[] key, int tagSizeInBytes) throws GeneralSecurityException {
    Validators.validateAesKeySize(key.length);

    if (tagSizeInBytes < MIN_TAG_SIZE_IN_BYTES) {
      throw new InvalidAlgorithmParameterException(
          "tag size too small, min is " + MIN_TAG_SIZE_IN_BYTES + " bytes");
    }
    if (tagSizeInBytes > AesUtil.BLOCK_SIZE) {
      throw new InvalidAlgorithmParameterException(
          "tag size too large, max is " + AesUtil.BLOCK_SIZE + " bytes");
    }

    keySpec = new SecretKeySpec(key, "AES");
    this.tagSizeInBytes = tagSizeInBytes;
    generateSubKeys();
  }

  // https://tools.ietf.org/html/rfc4493#section-2.4
  @Override
  public byte[] computeMac(final byte[] data) throws GeneralSecurityException {
    Cipher aes = instance();
    aes.init(Cipher.ENCRYPT_MODE, keySpec);

    // n is the number of blocks (including partial blocks) into which the data
    // is divided. Empty data is divided into 1 empty block.
    // Step 2: n = ceil(length / blocksize)
    // TODO(b/68969256): Adding a test that computes a CMAC of length 2**31-1.
    int n = Math.max(1, (int) Math.ceil((double) data.length / AesUtil.BLOCK_SIZE));

    // Step 3
    boolean flag = (n * AesUtil.BLOCK_SIZE == data.length);

    // Step 4
    byte[] mLast;
    if (flag) {
      mLast = Bytes.xor(data, (n - 1) * AesUtil.BLOCK_SIZE, subKey1, 0, AesUtil.BLOCK_SIZE);
    } else {
      mLast =
          Bytes.xor(
              AesUtil.cmacPad(Arrays.copyOfRange(data, (n - 1) * AesUtil.BLOCK_SIZE, data.length)),
              subKey2);
    }

    // Step 5
    byte[] x = new byte[AesUtil.BLOCK_SIZE];

    // Step 6
    byte[] y;
    for (int i = 0; i < n - 1; i++) {
      y = Bytes.xor(x, 0, data, i * AesUtil.BLOCK_SIZE, AesUtil.BLOCK_SIZE);
      x = aes.doFinal(y);
    }
    y = Bytes.xor(mLast, x);

    // Step 7
    byte[] tag = new byte[tagSizeInBytes];
    System.arraycopy(aes.doFinal(y), 0, tag, 0, tagSizeInBytes);
    return tag;
  }

  @Override
  public void verifyMac(final byte[] mac, byte[] data) throws GeneralSecurityException {
    if (!Bytes.equal(mac, this.computeMac(data))) {
      throw new GeneralSecurityException("invalid MAC");
    }
  }

  // https://tools.ietf.org/html/rfc4493#section-2.3
  private void generateSubKeys() throws GeneralSecurityException {
    Cipher aes = instance();
    aes.init(Cipher.ENCRYPT_MODE, keySpec);
    byte[] zeroes = new byte[AesUtil.BLOCK_SIZE];
    byte[] l = aes.doFinal(zeroes);
    subKey1 = AesUtil.dbl(l);
    subKey2 = AesUtil.dbl(subKey1);
  }
}
