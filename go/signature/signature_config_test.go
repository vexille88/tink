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

package signature_test

import (
	"testing"

	"github.com/google/tink/go/signature"
	"github.com/google/tink/go/tink"
)

func TestSignatureConfigRegistration(t *testing.T) {
	_, err := signature.RegisterStandardKeyTypes()
	if err != nil {
		t.Errorf("cannot register standard key types")
	}
	// check for EcdsaSignKeyManager
	keyManager, err := tink.GetKeyManager(signature.EcdsaSignTypeURL)
	if err != nil {
		t.Errorf("unexpected error: %s", err)
	}
	var _ = keyManager.(*signature.EcdsaSignKeyManager)

	// check for EcdsaVerifyKeyManager
	keyManager, err = tink.GetKeyManager(signature.EcdsaVerifyTypeURL)
	if err != nil {
		t.Errorf("unexpected error: %s", err)
	}
	var _ = keyManager.(*signature.EcdsaVerifyKeyManager)
}
