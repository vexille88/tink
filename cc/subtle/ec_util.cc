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

#include "tink/subtle/ec_util.h"

#include <memory>
#include <string>

#include "tink/subtle/common_enums.h"
#include "tink/subtle/subtle_util_boringssl.h"
#include "tink/util/errors.h"
#include "openssl/bn.h"
#include "openssl/ec.h"
#include "openssl/x509.h"

namespace crypto {
namespace tink {
namespace subtle {

// static
uint32_t EcUtil::FieldSizeInBytes(EllipticCurveType curve_type) {
  auto ec_group_result = SubtleUtilBoringSSL::GetEcGroup(curve_type);
  if (!ec_group_result.ok()) return 0;
  bssl::UniquePtr<EC_GROUP> ec_group(ec_group_result.ValueOrDie());
  return (EC_GROUP_get_degree(ec_group.get()) + 7) / 8;
}

// static
crypto::tink::util::StatusOr<uint32_t> EcUtil::EncodingSizeInBytes(
    EllipticCurveType curve_type, EcPointFormat point_format) {
  int coordinate_size = FieldSizeInBytes(curve_type);
  if (coordinate_size == 0) {
    return ToStatusF(crypto::tink::util::error::INVALID_ARGUMENT,
                     "Unsupported elliptic curve type: %s",
                     EnumToString(curve_type).c_str());
  }
  switch (point_format) {
  case EcPointFormat::UNCOMPRESSED:
    return 2 * coordinate_size + 1;
  case EcPointFormat::DO_NOT_USE_CRUNCHY_UNCOMPRESSED:
    return 2 * coordinate_size;
  case EcPointFormat::COMPRESSED:
    return coordinate_size + 1;
  default:
    return ToStatusF(crypto::tink::util::error::INVALID_ARGUMENT,
                     "Unsupported elliptic curve point format: %s",
                     EnumToString(point_format).c_str());
  }
}

}  // namespace subtle
}  // namespace tink
}  // namespace crypto
