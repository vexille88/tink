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

#include "tink/subtle/wycheproof_util.h"

#include <fstream>
#include <iostream>
#include <memory>

#include "absl/strings/string_view.h"
#include "include/rapidjson/document.h"
#include "include/rapidjson/istreamwrapper.h"
#include "tink/util/status.h"
#include "tink/util/statusor.h"
#include "tink/subtle/common_enums.h"

namespace crypto {
namespace tink {
namespace subtle {

namespace {

// TODO: factor these helpers out to an "util"-class.
util::StatusOr<std::string> HexDecode(absl::string_view hex) {
  if (hex.size() % 2 != 0) {
    return util::Status(util::error::INVALID_ARGUMENT, "Input has odd size.");
  }
  std::string decoded(hex.size() / 2, static_cast<char>(0));
  for (size_t i = 0; i < hex.size(); ++i) {
    char c = hex[i];
    char val;
    if ('0' <= c && c <= '9')
      val = c - '0';
    else if ('a' <= c && c <= 'f')
      val = c - 'a' + 10;
    else if ('A' <= c && c <= 'F')
      val = c - 'A' + 10;
    else
      return util::Status(util::error::INVALID_ARGUMENT, "Not hexadecimal");
    decoded[i / 2] = (decoded[i / 2] << 4) | val;
  }
  return decoded;
}

std::string HexDecodeOrDie(absl::string_view hex) {
  return HexDecode(hex).ValueOrDie();
}

}  // namespace

std::string WycheproofUtil::GetBytes(const rapidjson::Value &val) {
  std::string s(val.GetString());
  if (s.size() % 2 != 0) {
    // ECDH private key may have odd length.
    s = "0" + s;
  }
  return HexDecodeOrDie(s);
}

std::unique_ptr<rapidjson::Document>
WycheproofUtil::ReadTestVectors(const std::string &filename) {
  const std::string kTestVectors = "../wycheproof/testvectors/";
  std::ifstream input_stream;
  input_stream.open(kTestVectors + filename);
  rapidjson::IStreamWrapper input(input_stream);
  std::unique_ptr<rapidjson::Document> root(
      new rapidjson::Document(rapidjson::kObjectType));
  if (root->ParseStream(input).HasParseError()) {
    std::cerr << "Failure parsing of test vectors from "
              << kTestVectors + filename << "\n";
    exit(1);
  }
  return root;
}

HashType WycheproofUtil::GetHashType(const rapidjson::Value &val) {
  std::string md(val.GetString());
  if (md == "SHA-1") {
    return HashType::SHA1;
  } else if (md == "SHA-256") {
    return HashType::SHA256;
  } else if (md == "SHA-384") {
    return HashType::UNKNOWN_HASH;
  } else if (md == "SHA-512") {
    return HashType::SHA512;
  } else {
    return HashType::UNKNOWN_HASH;
  }
}

EllipticCurveType
WycheproofUtil::GetEllipticCurveType(const rapidjson::Value &val) {
  std::string curve(val.GetString());
  if (curve == "secp256r1") {
    return EllipticCurveType::NIST_P256;
  } else if (curve == "secp384r1") {
    return EllipticCurveType::NIST_P384;
  } else if (curve == "secp521r1") {
    return EllipticCurveType::NIST_P521;
  } else {
    return EllipticCurveType::UNKNOWN_CURVE;
  }
}

}  // namespace subtle
}  // namespace tink
}  // namespace crypto
