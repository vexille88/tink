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

package mac_test

import (
	"fmt"
	"strings"
	"testing"

	"github.com/google/tink/go/mac"
	"github.com/google/tink/go/testutil"
	"github.com/google/tink/go/tink"
	tinkpb "github.com/google/tink/proto/tink_go_proto"
)

func TestFactoryMultipleKeys(t *testing.T) {
	tagSize := uint32(16)
	keyset := testutil.NewTestHmacKeyset(tagSize, tinkpb.OutputPrefixType_TINK)
	primaryKey := keyset.Key[0]
	if primaryKey.OutputPrefixType != tinkpb.OutputPrefixType_TINK {
		t.Errorf("expect a tink key")
	}
	keysetHandle, _ := tink.CleartextKeysetHandle().ParseKeyset(keyset)

	p, err := mac.GetPrimitive(keysetHandle)
	if err != nil {
		t.Errorf("GetPrimitive failed: %s", err)
	}
	expectedPrefix, _ := tink.GetOutputPrefix(primaryKey)
	if err := verifyMacPrimitive(p, p, expectedPrefix, tagSize); err != nil {
		t.Errorf("invalid primitive: %s", err)
	}

	// mac with a primary RAW key, verify with the keyset
	rawKey := keyset.Key[1]
	if rawKey.OutputPrefixType != tinkpb.OutputPrefixType_RAW {
		t.Errorf("expect a raw key")
	}
	keyset2 := tink.CreateKeyset(rawKey.KeyId, []*tinkpb.Keyset_Key{rawKey})
	keysetHandle2, _ := tink.CleartextKeysetHandle().ParseKeyset(keyset2)
	p2, err := mac.GetPrimitive(keysetHandle2)
	if err != nil {
		t.Errorf("GetPrimitive failed: %s", err)
	}
	if err := verifyMacPrimitive(p2, p, tink.RawPrefix, tagSize); err != nil {
		t.Errorf("invalid primitive: %s", err)
	}

	// mac with a random key not in the keyset, verify with the keyset should fail
	keyset2 = testutil.NewTestHmacKeyset(tagSize, tinkpb.OutputPrefixType_TINK)
	primaryKey = keyset2.Key[0]
	expectedPrefix, _ = tink.GetOutputPrefix(primaryKey)
	keysetHandle2, _ = tink.CleartextKeysetHandle().ParseKeyset(keyset2)
	p2, err = mac.GetPrimitive(keysetHandle2)
	if err != nil {
		t.Errorf("cannot get primitive from keyset handle")
	}
	err = verifyMacPrimitive(p2, p, expectedPrefix, tagSize)
	if err == nil || !strings.Contains(err.Error(), "mac verification failed") {
		t.Errorf("Invalid MAC, shouldn't return valid")
	}
}

func TestFactoryRawKey(t *testing.T) {
	tagSize := uint32(16)
	keyset := testutil.NewTestHmacKeyset(tagSize, tinkpb.OutputPrefixType_RAW)
	primaryKey := keyset.Key[0]
	if primaryKey.OutputPrefixType != tinkpb.OutputPrefixType_RAW {
		t.Errorf("expect a raw key")
	}
	keysetHandle, _ := tink.CleartextKeysetHandle().ParseKeyset(keyset)
	p, err := mac.GetPrimitive(keysetHandle)
	if err != nil {
		t.Errorf("GetPrimitive failed: %s", err)
	}
	if err := verifyMacPrimitive(p, p, tink.RawPrefix, tagSize); err != nil {
		t.Errorf("invalid primitive: %s", err)
	}
}

func verifyMacPrimitive(computePrimitive tink.Mac, verifyPrimitive tink.Mac,
	expectedPrefix string, tagSize uint32) error {
	data := []byte("hello")
	tag, err := computePrimitive.ComputeMac(data)
	if err != nil {
		return fmt.Errorf("mac computation failed: %s", err)
	}
	prefixSize := len(expectedPrefix)
	if string(tag[:prefixSize]) != expectedPrefix {
		return fmt.Errorf("incorrect prefix")
	}
	if prefixSize+int(tagSize) != len(tag) {
		return fmt.Errorf("incorrect tag length")
	}
	valid, err := verifyPrimitive.VerifyMac(tag, data)
	if !valid || err != nil {
		return fmt.Errorf("mac verification failed: %s", err)
	}

	// Modify plaintext or tag and make sure VerifyMac failed.
	var dataAndTag []byte
	dataAndTag = append(dataAndTag, data...)
	dataAndTag = append(dataAndTag, tag...)
	valid, err = verifyPrimitive.VerifyMac(dataAndTag[len(data):], dataAndTag[:len(data)])
	if !valid || err != nil {
		return fmt.Errorf("mac verification failed: %s", err)
	}
	for i := 0; i < len(dataAndTag); i++ {
		tmp := dataAndTag[i]
		for j := 0; j < 8; j++ {
			dataAndTag[i] ^= 1 << uint8(j)
			valid, err = verifyPrimitive.VerifyMac(dataAndTag[len(data):], dataAndTag[:len(data)])
			if valid && err == nil {
				return fmt.Errorf("invalid tag or plaintext, mac should be invalid")
			}
			dataAndTag[i] = tmp
		}
	}
	return nil
}
