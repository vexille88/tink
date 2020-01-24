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

package aead_test

import (
	"bytes"
	"fmt"
	"strings"
	"testing"

	"github.com/google/tink/go/aead"
	subtleAead "github.com/google/tink/go/subtle/aead"
	"github.com/google/tink/go/subtle/random"
	"github.com/google/tink/go/testutil"
	"github.com/google/tink/go/tink"
	tinkpb "github.com/google/tink/proto/tink_go_proto"
)

func setupFactoryTest() {
	aead.RegisterStandardKeyTypes()
}

func TestFactoryMultipleKeys(t *testing.T) {
	setupFactoryTest()
	// encrypt with non-raw key
	keyset := testutil.NewTestAesGcmKeyset(tinkpb.OutputPrefixType_TINK)
	primaryKey := keyset.Key[0]
	if primaryKey.OutputPrefixType == tinkpb.OutputPrefixType_RAW {
		t.Errorf("expect a non-raw key")
	}
	keysetHandle, _ := tink.CleartextKeysetHandle().ParseKeyset(keyset)
	a, err := aead.GetPrimitive(keysetHandle)
	if err != nil {
		t.Errorf("GetPrimitive failed: %s", err)
	}
	expectedPrefix, _ := tink.GetOutputPrefix(primaryKey)
	if err := validateAeadFactoryCipher(a, a, expectedPrefix); err != nil {
		t.Errorf("invalid cipher: %s", err)
	}

	// encrypt with a non-primary RAW key and decrypt with the keyset
	rawKey := keyset.Key[1]
	if rawKey.OutputPrefixType != tinkpb.OutputPrefixType_RAW {
		t.Errorf("expect a raw key")
	}
	keyset2 := tink.CreateKeyset(rawKey.KeyId, []*tinkpb.Keyset_Key{rawKey})
	keysetHandle2, _ := tink.CleartextKeysetHandle().ParseKeyset(keyset2)
	a2, err := aead.GetPrimitive(keysetHandle2)
	if err != nil {
		t.Errorf("GetPrimitive failed: %s", err)
	}
	if err := validateAeadFactoryCipher(a2, a, tink.RawPrefix); err != nil {
		t.Errorf("invalid cipher: %s", err)
	}

	// encrypt with a random key not in the keyset, decrypt with the keyset should fail
	keyset2 = testutil.NewTestAesGcmKeyset(tinkpb.OutputPrefixType_TINK)
	primaryKey = keyset2.Key[0]
	expectedPrefix, _ = tink.GetOutputPrefix(primaryKey)
	keysetHandle2, _ = tink.CleartextKeysetHandle().ParseKeyset(keyset2)
	a2, err = aead.GetPrimitive(keysetHandle2)
	if err != nil {
		t.Errorf("GetPrimitive failed: %s", err)
	}
	err = validateAeadFactoryCipher(a2, a, expectedPrefix)
	if err == nil || !strings.Contains(err.Error(), "decryption failed") {
		t.Errorf("expect decryption to fail with random key: %s", err)
	}
}

func TestFactoryRawKeyAsPrimary(t *testing.T) {
	setupFactoryTest()
	keyset := testutil.NewTestAesGcmKeyset(tinkpb.OutputPrefixType_RAW)
	if keyset.Key[0].OutputPrefixType != tinkpb.OutputPrefixType_RAW {
		t.Errorf("primary key is not a raw key")
	}
	keysetHandle, _ := tink.CleartextKeysetHandle().ParseKeyset(keyset)

	a, err := aead.GetPrimitive(keysetHandle)
	if err != nil {
		t.Errorf("cannot get primitive from keyset handle: %s", err)
	}
	if err := validateAeadFactoryCipher(a, a, tink.RawPrefix); err != nil {
		t.Errorf("invalid cipher: %s", err)
	}
}

func validateAeadFactoryCipher(encryptCipher tink.Aead,
	decryptCipher tink.Aead,
	expectedPrefix string) error {
	prefixSize := len(expectedPrefix)
	// regular plaintext
	pt := random.GetRandomBytes(20)
	ad := random.GetRandomBytes(20)
	ct, err := encryptCipher.Encrypt(pt, ad)
	if err != nil {
		return fmt.Errorf("encryption failed with regular plaintext: %s", err)
	}
	decrypted, err := decryptCipher.Decrypt(ct, ad)
	if err != nil || !bytes.Equal(decrypted, pt) {
		return fmt.Errorf("decryption failed with regular plaintext: err: %s, pt: %s, decrypted: %s",
			err, pt, decrypted)
	}
	if string(ct[:prefixSize]) != expectedPrefix {
		return fmt.Errorf("incorrect prefix with regular plaintext")
	}
	if prefixSize+len(pt)+subtleAead.AesGcmIvSize+subtleAead.AesGcmTagSize != len(ct) {
		return fmt.Errorf("lengths of plaintext and ciphertext don't match with regular plaintext")
	}

	// short plaintext
	pt = random.GetRandomBytes(1)
	ct, err = encryptCipher.Encrypt(pt, ad)
	if err != nil {
		return fmt.Errorf("encryption failed with short plaintext: %s", err)
	}
	decrypted, err = decryptCipher.Decrypt(ct, ad)
	if err != nil || !bytes.Equal(decrypted, pt) {
		return fmt.Errorf("decryption failed with short plaintext: err: %s, pt: %s, decrypted: %s",
			err, pt, decrypted)
	}
	if string(ct[:prefixSize]) != expectedPrefix {
		return fmt.Errorf("incorrect prefix with short plaintext")
	}
	if prefixSize+len(pt)+subtleAead.AesGcmIvSize+subtleAead.AesGcmTagSize != len(ct) {
		return fmt.Errorf("lengths of plaintext and ciphertext don't match with short plaintext")
	}
	return nil
}
