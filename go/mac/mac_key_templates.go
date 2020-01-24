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

package mac

import (
	"github.com/golang/protobuf/proto"
	commonpb "github.com/google/tink/proto/common_go_proto"
	hmacpb "github.com/google/tink/proto/hmac_go_proto"
	tinkpb "github.com/google/tink/proto/tink_go_proto"
)

// This file contains pre-generated KeyTemplate for MAC.

// HmacSha256Tag128KeyTemplate is a KeyTemplate for HmacKey with the following
// parameters:
//   - Key size: 32 bytes
//   - Tag size: 16 bytes
//   - Hash function: SHA256
func HmacSha256Tag128KeyTemplate() *tinkpb.KeyTemplate {
	return createHmacKeyTemplate(32, 16, commonpb.HashType_SHA256)
}

// HmacSha256Tag256KeyTemplate is a KeyTemplate for HmacKey with the following
// parameters:
//   - Key size: 32 bytes
//   - Tag size: 32 bytes
//   - Hash function: SHA256
func HmacSha256Tag256KeyTemplate() *tinkpb.KeyTemplate {
	return createHmacKeyTemplate(32, 32, commonpb.HashType_SHA256)
}

// createHmacKeyTemplate creates a new KeyTemplate for Hmac using the given parameters.
func createHmacKeyTemplate(keySize uint32,
	tagSize uint32,
	hashType commonpb.HashType) *tinkpb.KeyTemplate {
	params := hmacpb.HmacParams{
		Hash:    hashType,
		TagSize: tagSize,
	}
	format := hmacpb.HmacKeyFormat{
		Params:  &params,
		KeySize: keySize,
	}
	serializedFormat, _ := proto.Marshal(&format)
	return &tinkpb.KeyTemplate{
		TypeUrl: HmacTypeURL,
		Value:   serializedFormat,
	}
}
