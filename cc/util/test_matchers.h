// Copyright 2018 Google Inc.
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

#ifndef TINK_CC_UTIL_TEST_MATCHERS_H_
#define TINK_CC_UTIL_TEST_MATCHERS_H_

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "tink/util/status.h"

namespace crypto {
namespace tink {
namespace test {

// Matches a util::Status value. This is better than
// EXPECT_TRUE(status.ok())
// because the error message is a part of the failure messsage.
MATCHER(IsOk, "is a Status with an OK value") {
  if (arg.ok()) {
    return true;
  }
  *result_listener << arg.ToString();
  return false;
}

MATCHER_P(StatusIs, code,
          "is a Status with a " + util::ErrorCodeString(code) + " code") {
  if (arg.CanonicalCode() == code) {
    return true;
  }
  *result_listener << ::testing::PrintToString(arg);
  return false;
}

}  // namespace test
}  // namespace tink
}  // namespace crypto

#endif  // TINK_CC_UTIL_TEST_MATCHERS_H_
