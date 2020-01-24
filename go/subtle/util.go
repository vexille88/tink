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

// Package subtle provides utility functions.
package subtle

import (
	"crypto/elliptic"
	"crypto/sha1"
	"crypto/sha256"
	"crypto/sha512"
	"encoding/hex"
	"hash"
	"math/big"
)

// ConvertHashName converts different forms of a hash name to the
// hash name that tink recognizes.
func ConvertHashName(name string) string {
	switch name {
	case "SHA-256":
		return "SHA256"
	case "SHA-512":
		return "SHA512"
	case "SHA-1":
		return "SHA1"
	default:
		return ""
	}
}

// ConvertCurveName converts different forms of a curve name to the
// name that tink recognizes
func ConvertCurveName(name string) string {
	switch name {
	case "secp256r1", "P-256":
		return "NIST_P256"
	case "secp384r1", "P-384":
		return "NIST_P384"
	case "secp521r1", "P-521":
		return "NIST_P521"
	default:
		return ""
	}
}

// GetHashFunc returns the corresponding hash function of the given hash name.
func GetHashFunc(hash string) func() hash.Hash {
	switch hash {
	case "SHA1":
		return sha1.New
	case "SHA256":
		return sha256.New
	case "SHA512":
		return sha512.New
	default:
		return nil
	}
}

// GetCurve returns the curve object that corresponds to the given curve type.
// It returns null if the curve type is not supported.
func GetCurve(curve string) elliptic.Curve {
	switch curve {
	case "NIST_P256":
		return elliptic.P256()
	case "NIST_P384":
		return elliptic.P384()
	case "NIST_P521":
		return elliptic.P521()
	default:
		return nil
	}
}

// ComputeHash calculates a hash of the given data using the given hash function.
func ComputeHash(hashFunc func() hash.Hash, data []byte) []byte {
	h := hashFunc()
	h.Write(data)
	ret := h.Sum(nil)
	return ret
}

// NewBigIntFromHex returns a big integer from a hex string.
func NewBigIntFromHex(s string) (*big.Int, error) {
	if len(s)%2 == 1 {
		s = "0" + s
	}
	b, err := hex.DecodeString(s)
	if err != nil {
		return nil, err
	}
	ret := new(big.Int).SetBytes(b)
	return ret, nil
}
