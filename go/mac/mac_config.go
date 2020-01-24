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

// Package mac provides implementations of the Mac primitive.
package mac

import (
	"github.com/google/tink/go/tink"
)

// RegisterStandardKeyTypes registers standard Mac key types and their managers
// with the Registry.
func RegisterStandardKeyTypes() (bool, error) {
	return RegisterKeyManager(NewHmacKeyManager())
}

// RegisterKeyManager registers the given keyManager for the key type given in
// keyManager.KeyType(). It returns true if registration was successful, false if
// there already exisits a key manager for the key type.
func RegisterKeyManager(keyManager tink.KeyManager) (bool, error) {
	return tink.RegisterKeyManager(keyManager)
}
