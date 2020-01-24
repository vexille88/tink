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

#ifndef TINK_PRIMITIVE_SET_H_
#define TINK_PRIMITIVE_SET_H_

#include <mutex>  // NOLINT(build/c++11)
#include <unordered_map>
#include <vector>

#include "absl/memory/memory.h"
#include "tink/crypto_format.h"
#include "tink/util/errors.h"
#include "tink/util/statusor.h"
#include "proto/tink.pb.h"

namespace crypto {
namespace tink {

// A container class for a set of primitives (i.e. implementations of
// cryptographic primitives offered by Tink).  It provides also
// additional properties for the primitives it holds.  In particular,
// one of the primitives in the set can be distinguished as "the
// primary" one.
//
// PrimitiveSet is an auxiliary class used for supporting key rotation:
// primitives in a set correspond to keys in a keyset.  Users will
// usually work with primitive instances, which essentially wrap
// primitive sets.  For example an instance of an Aead-primitive for a
// given keyset holds a set of Aead-primitivies corresponding to the
// keys in the keyset, and uses the set members to do the actual
// crypto operations: to encrypt data the primary Aead-primitive from
// the set is used, and upon decryption the ciphertext's prefix
// determines the identifier of the primitive from the set.
//
// PrimitiveSet is a public class to allow its use in implementations
// of custom primitives.
template <class P>
class PrimitiveSet {
 public:
  // Entry-objects hold individual instances of primitives in the set.
  template <class P2>
  class Entry {
   public:
    Entry(std::unique_ptr<P2> primitive, const std::string& identifier,
          google::crypto::tink::KeyStatusType status,
          google::crypto::tink::OutputPrefixType output_prefix_type)
        : primitive_(std::move(primitive)),
          identifier_(identifier),
          status_(status),
          output_prefix_type_(output_prefix_type) {}

    P2& get_primitive() const { return *primitive_; }

    const std::string& get_identifier() const { return identifier_; }

    const google::crypto::tink::KeyStatusType get_status() const {
      return status_;
    }

    const google::crypto::tink::OutputPrefixType get_output_prefix_type()
        const {
      return output_prefix_type_;
    }

   private:
    std::unique_ptr<P> primitive_;
    std::string identifier_;
    google::crypto::tink::KeyStatusType status_;
    google::crypto::tink::OutputPrefixType output_prefix_type_;
  };

  typedef std::vector<std::unique_ptr<Entry<P>>> Primitives;

  // Constructs an empty PrimitiveSet.
  PrimitiveSet<P>() : primary_(nullptr) {}

  // Adds 'primitive' to this set for the specified 'key'.
  crypto::tink::util::StatusOr<Entry<P>*> AddPrimitive(
      std::unique_ptr<P> primitive, google::crypto::tink::Keyset::Key key) {
    auto identifier_result = CryptoFormat::get_output_prefix(key);
    if (!identifier_result.ok()) return identifier_result.status();
    if (primitive == nullptr) {
      return ToStatusF(crypto::tink::util::error::INVALID_ARGUMENT,
                       "The primitive must be non-null.");
    }
    std::string identifier = identifier_result.ValueOrDie();
    std::lock_guard<std::mutex> lock(primitives_mutex_);
    primitives_[identifier].push_back(
        absl::make_unique<Entry<P>>(std::move(primitive),
                                    identifier, key.status(),
                                    key.output_prefix_type()));
    return primitives_[identifier].back().get();
  }

  // Returns the entries with primitives identifed by 'identifier'.
  crypto::tink::util::StatusOr<const Primitives*> get_primitives(
      const std::string& identifier) {
    std::lock_guard<std::mutex> lock(primitives_mutex_);
    typename CiphertextPrefixToPrimitivesMap::iterator found =
        primitives_.find(identifier);
    if (found == primitives_.end()) {
      return ToStatusF(crypto::tink::util::error::NOT_FOUND,
                       "No primitives found for identifier '%s'.",
                       identifier.c_str());
    }
    return &(found->second);
  }

  // Returns all primitives that use RAW prefix.
  crypto::tink::util::StatusOr<const Primitives*> get_raw_primitives() {
    return get_primitives(CryptoFormat::kRawPrefix);
  }

  // Sets the given 'primary' as as the primary primitive of this set.
  void set_primary(Entry<P>* primary) { primary_ = primary; }

  // Returns the entry with the primary primitive.
  const Entry<P>* get_primary() const { return primary_; }

 private:
  typedef std::unordered_map<std::string, Primitives>
      CiphertextPrefixToPrimitivesMap;
  Entry<P>* primary_;  // the Entry<P> object is owned by primitives_
  std::mutex primitives_mutex_;
  CiphertextPrefixToPrimitivesMap primitives_;  // guarded by primitives_mutex_
};

}  // namespace tink
}  // namespace crypto

#endif  // TINK_PRIMITIVE_SET_H_
