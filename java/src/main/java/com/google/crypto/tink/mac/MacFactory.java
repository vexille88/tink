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

package com.google.crypto.tink.mac;

import com.google.crypto.tink.CryptoFormat;
import com.google.crypto.tink.KeyManager;
import com.google.crypto.tink.KeysetHandle;
import com.google.crypto.tink.Mac;
import com.google.crypto.tink.PrimitiveSet;
import com.google.crypto.tink.Registry;
import com.google.crypto.tink.proto.OutputPrefixType;
import com.google.crypto.tink.subtle.Bytes;
import java.security.GeneralSecurityException;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.logging.Logger;

/**
 * Static methods for obtaining {@link Mac} instances.
 *
 * <h3>Usage</h3>
 *
 * <pre>{@code
 * KeysetHandle keysetHandle = ...;
 * Mac mac = MacFactory.getPrimitive(keysetHandle);
 * byte[] data = ...;
 * byte[] tag = mac.computeMac(data);
 *
 * }</pre>
 *
 * <p>The returned primitive works with a keyset (rather than a single key). To compute a MAC tag,
 * it uses the primary key in the keyset, and prepends to the tag a certain prefix associated with
 * the primary key. To verify a tag, the primitive uses the prefix of the tag to efficiently select
 * the right key in the set. If the keys associated with the prefix do not validate the tag, the
 * primitive tries all keys with {@link com.google.crypto.tink.proto.OutputPrefixType#RAW}.
 *
 * @since 1.0.0
 */
public final class MacFactory {
  private static final Logger logger = Logger.getLogger(MacFactory.class.getName());

  /**
   * @return a Mac primitive from a {@code keysetHandle}.
   * @throws GeneralSecurityException
   */
  public static Mac getPrimitive(KeysetHandle keysetHandle) throws GeneralSecurityException {
    return getPrimitive(keysetHandle, /* keyManager= */ null);
  }

  /**
   * @return a Mac primitive from a {@code keysetHandle} and a custom {@code keyManager}.
   * @throws GeneralSecurityException
   */
  public static Mac getPrimitive(KeysetHandle keysetHandle, final KeyManager<Mac> keyManager)
      throws GeneralSecurityException {
    final PrimitiveSet<Mac> primitives = Registry.getPrimitives(keysetHandle, keyManager);
    validate(primitives);
    final byte[] formatVersion = new byte[] {CryptoFormat.LEGACY_START_BYTE};
    return new Mac() {
      @Override
      public byte[] computeMac(final byte[] data) throws GeneralSecurityException {
        if (primitives.getPrimary().getOutputPrefixType().equals(OutputPrefixType.LEGACY)) {
          return Bytes.concat(
              primitives.getPrimary().getIdentifier(),
              primitives.getPrimary().getPrimitive().computeMac(Bytes.concat(data, formatVersion)));
        }
        return Bytes.concat(
            primitives.getPrimary().getIdentifier(),
            primitives.getPrimary().getPrimitive().computeMac(data));
      }

      @Override
      public void verifyMac(final byte[] mac, final byte[] data) throws GeneralSecurityException {
        if (mac.length <= CryptoFormat.NON_RAW_PREFIX_SIZE) {
          // This also rejects raw MAC with size of 4 bytes or fewer. Those MACs are
          // clearly insecure, thus should be discouraged.
          throw new GeneralSecurityException("tag too short");
        }
        byte[] prefix = Arrays.copyOfRange(mac, 0, CryptoFormat.NON_RAW_PREFIX_SIZE);
        byte[] macNoPrefix = Arrays.copyOfRange(mac, CryptoFormat.NON_RAW_PREFIX_SIZE, mac.length);
        List<PrimitiveSet.Entry<Mac>> entries = primitives.getPrimitive(prefix);
        for (PrimitiveSet.Entry<Mac> entry : entries) {
          try {
            if (entry.getOutputPrefixType().equals(OutputPrefixType.LEGACY)) {
              entry.getPrimitive().verifyMac(macNoPrefix, Bytes.concat(data, formatVersion));
            } else {
              entry.getPrimitive().verifyMac(macNoPrefix, data);
            }
            // If there is no exception, the MAC is valid and we can return.
            return;
          } catch (GeneralSecurityException e) {
            logger.info("tag prefix matches a key, but cannot verify: " + e.toString());
            // Ignored as we want to continue verification with the remaining keys.
          }
        }

        // None "non-raw" key matched, so let's try the raw keys (if any exist).
        entries = primitives.getRawPrimitives();
        for (PrimitiveSet.Entry<Mac> entry : entries) {
          try {
            entry.getPrimitive().verifyMac(mac, data);
            // If there is no exception, the MAC is valid and we can return.
            return;
          } catch (GeneralSecurityException ignored) {
            // Ignored as we want to continue verification with other raw keys.
          }
        }
        // nothing works.
        throw new GeneralSecurityException("invalid MAC");
      }
    };
  }

  // Check that all primitives in <code>pset</code> are Mac instances.
  private static void validate(final PrimitiveSet<Mac> pset) throws GeneralSecurityException {
    for (Collection<PrimitiveSet.Entry<Mac>> entries : pset.getAll()) {
      for (PrimitiveSet.Entry<Mac> entry : entries) {
        if (!(entry.getPrimitive() instanceof Mac)) {
          throw new GeneralSecurityException("invalid MAC key material");
        }
      }
    }
  }
}
