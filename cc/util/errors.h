// Copyright 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef TINK_UTIL_ERRORS_H_
#define TINK_UTIL_ERRORS_H_

#include "tink/util/status.h"

// from #include "absl/base/port.h"
#define PRINTF_ATTRIBUTE(string_index, first_to_check)                  \
    __attribute__((__format__ (__printf__, string_index, first_to_check)))

namespace crypto {
namespace tink {

// Constructs a Status object given a printf-style va list.
crypto::tink::util::Status ToStatusF(
    crypto::tink::util::error::Code code, const char* format, ...)
    PRINTF_ATTRIBUTE(2, 3);

}  // namespace tink
}  // namespace crypto

#endif  // TINK_UTIL_ERRORS_H_
