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
	commonpb "github.com/google/tink/proto/common_go_proto"
	hmacpb "github.com/google/tink/proto/hmac_go_proto"
)

// NewHmacParams returns a new HmacParams.
func NewHmacParams(hashType commonpb.HashType, tagSize uint32) *hmacpb.HmacParams {
	return &hmacpb.HmacParams{
		Hash:    hashType,
		TagSize: tagSize,
	}
}

// NewHmacKey returns a new HmacKey.
func NewHmacKey(params *hmacpb.HmacParams, version uint32, keyValue []byte) *hmacpb.HmacKey {
	return &hmacpb.HmacKey{
		Version:  version,
		Params:   params,
		KeyValue: keyValue,
	}
}

// NewHmacKeyFormat returns a new HmacKeyFormat.
func NewHmacKeyFormat(params *hmacpb.HmacParams, keySize uint32) *hmacpb.HmacKeyFormat {
	return &hmacpb.HmacKeyFormat{
		Params:  params,
		KeySize: keySize,
	}
}
