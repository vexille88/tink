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

#include <sstream>

#include "tink/util/status.h"
// placeholder_google3_status_header, please ignore

using ::std::ostream;

namespace crypto {
namespace tink {
namespace util {

namespace {


const Status& GetCancelled() {
  static const Status status(::crypto::tink::util::error::CANCELLED, "");
  return status;
}

const Status& GetUnknown() {
  static const Status status(::crypto::tink::util::error::UNKNOWN, "");
  return status;
}


}  // namespace

// placeholder_implicit_type_conversion, please ignore

Status::Status() : code_(::crypto::tink::util::error::OK), message_("") {
}

Status::Status(::crypto::tink::util::error::Code error,
               const std::string& error_message)
    : code_(error), message_(error_message) {
  if (code_ == ::crypto::tink::util::error::OK) {
    message_.clear();
  }
}

Status& Status::operator=(const Status& other) {
  code_ = other.code_;
  message_ = other.message_;
  return *this;
}

const Status& Status::CANCELLED = GetCancelled();
const Status& Status::UNKNOWN = GetUnknown();
const Status& Status::OK = Status();

std::string Status::ToString() const {
  if (code_ == ::crypto::tink::util::error::OK) {
    return "OK";
  }

  std::ostringstream oss;
  oss << code_ << ": " << message_;
  return oss.str();
}

std::string ErrorCodeString(crypto::tink::util::error::Code error) {
  switch (error) {
    case crypto::tink::util::error::OK:
      return "OK";
    case crypto::tink::util::error::CANCELLED:
      return "CANCELLED";
    case crypto::tink::util::error::UNKNOWN:
      return "UNKNOWN";
    case crypto::tink::util::error::INVALID_ARGUMENT:
      return "INVALID_ARGUMENT";
    case crypto::tink::util::error::DEADLINE_EXCEEDED:
      return "DEADLINE_EXCEEDED";
    case crypto::tink::util::error::NOT_FOUND:
      return "NOT_FOUND";
    case crypto::tink::util::error::ALREADY_EXISTS:
      return "ALREADY_EXISTS";
    case crypto::tink::util::error::PERMISSION_DENIED:
      return "PERMISSION_DENIED";
    case crypto::tink::util::error::RESOURCE_EXHAUSTED:
      return "RESOURCE_EXHAUSTED";
    case crypto::tink::util::error::FAILED_PRECONDITION:
      return "FAILED_PRECONDITION";
    case crypto::tink::util::error::ABORTED:
      return "ABORTED";
    case crypto::tink::util::error::OUT_OF_RANGE:
      return "OUT_OF_RANGE";
    case crypto::tink::util::error::UNIMPLEMENTED:
      return "UNIMPLEMENTED";
    case crypto::tink::util::error::INTERNAL:
      return "INTERNAL";
    case crypto::tink::util::error::UNAVAILABLE:
      return "UNAVAILABLE";
    case crypto::tink::util::error::DATA_LOSS:
      return "DATA_LOSS";
  }
  // Avoid using a "default" in the switch, so that the compiler can
  // give us a warning, but still provide a fallback here.
  return std::to_string(error);
}

extern ostream& operator<<(ostream& os, crypto::tink::util::error::Code code) {
  os << ErrorCodeString(code);
  return os;
}

extern ostream& operator<<(ostream& os, const Status& other) {
  os << other.ToString();
  return os;
}


}  // namespace util
}  // namespace tink
}  // namespace crypto
