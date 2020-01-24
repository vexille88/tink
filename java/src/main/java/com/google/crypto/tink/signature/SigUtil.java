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

package com.google.crypto.tink.signature;

import com.google.crypto.tink.proto.EcdsaParams;
import com.google.crypto.tink.proto.EcdsaSignatureEncoding;
import com.google.crypto.tink.proto.EllipticCurveType;
import com.google.crypto.tink.proto.HashType;
import com.google.crypto.tink.subtle.EllipticCurves;
import com.google.crypto.tink.subtle.Enums;
import java.security.GeneralSecurityException;

final class SigUtil {
  static final String INVALID_PARAMS = "Invalid ECDSA parameters";

  /**
   * Validates Ecdsa's parameters. The hash's strength must not be weaker than the curve's strength.
   *
   * @param params the Ecdsa's parameters protocol buffer.
   * @throws GeneralSecurityException iff it's invalid.
   */
  public static void validateEcdsaParams(EcdsaParams params) throws GeneralSecurityException {
    EcdsaSignatureEncoding encoding = params.getEncoding();
    HashType hash = params.getHashType();
    EllipticCurveType curve = params.getCurve();
    switch (encoding) {
      case DER: // fall through
      case IEEE_P1363:
        break;
      default:
        throw new GeneralSecurityException("unsupported signature encoding");
    }
    switch (curve) {
      case NIST_P256:
        // Using SHA512 for curve P256 is fine. However, only the 256 leftmost bits of the hash is
        // used in signature computation. Therefore, we don't allow it here to prevent security's
        // illusion.
        if (hash != HashType.SHA256) {
          throw new GeneralSecurityException(INVALID_PARAMS);
        }
        break;
      case NIST_P384:
        /* fall through */
      case NIST_P521:
        if (hash != HashType.SHA512) {
          throw new GeneralSecurityException(INVALID_PARAMS);
        }
        break;
      default:
        throw new GeneralSecurityException(INVALID_PARAMS);
    }
  }

  /** Converts protobuf enum {@code HashType} to raw Java enum {@code Enums.HashType}. */
  public static Enums.HashType toHashType(HashType hash) throws GeneralSecurityException {
    switch (hash) {
      case SHA1:
        return Enums.HashType.SHA1;
      case SHA256:
        return Enums.HashType.SHA256;
      case SHA512:
        return Enums.HashType.SHA512;
      default:
        throw new GeneralSecurityException("unknown hash type: " + hash);
    }
  }

  /** Converts protobuf enum {@code EllipticCurveType} to raw Java enum {code CurveType}. */
  public static EllipticCurves.CurveType toCurveType(EllipticCurveType type)
      throws GeneralSecurityException {
    switch (type) {
      case NIST_P256:
        return EllipticCurves.CurveType.NIST_P256;
      case NIST_P384:
        return EllipticCurves.CurveType.NIST_P384;
      case NIST_P521:
        return EllipticCurves.CurveType.NIST_P521;
      default:
        throw new GeneralSecurityException("unknown curve type: " + type);
    }
  }

  /**
   * Converts protobuf enum {@code EcdsaSignatureEncoding} to raw Java enum {code
   * EllipticCurves.EcdsaEncoding}.
   */
  public static EllipticCurves.EcdsaEncoding toEcdsaEncoding(EcdsaSignatureEncoding encoding)
      throws GeneralSecurityException {
    switch (encoding) {
      case IEEE_P1363:
        return EllipticCurves.EcdsaEncoding.IEEE_P1363;
      case DER:
        return EllipticCurves.EcdsaEncoding.DER;
      default:
        throw new GeneralSecurityException("unknown ECDSA encoding: " + encoding);
    }
  }
}
