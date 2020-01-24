# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
################################################################################

#!/bin/bash

ROOT_DIR="$TEST_SRCDIR/tink"
CC_KEYSET_RW_CLI="$ROOT_DIR/tools/testing/cc/keyset_reader_writer_cli"
JAVA_KEYSET_RW_CLI="$ROOT_DIR/tools/tinkey/tinkey"
TEST_UTIL="$ROOT_DIR/tools/testing/cross_language/test_util.sh"

source $TEST_UTIL || exit 1

#############################################################################
### Helpers for KeysetReader and KeysetWriter-tests.

# Checks cross-language compatibility of implementations of
# KeysetReader and KeysetWriter interfaces.
#
# Expects as parameter an input keyset in format given by 'input_format'
# (generated by the corresponding Java KeysetWriter).
# Uses CC_KEYSET_RW_CLI (which is based on C++ KeysetReader/Writer)
# to read the input and output a copy of the input keyst in format
# specified in 'output_format'.
keyset_reader_writer_basic_test() {
  local test_name="keyset-reader-writer-basic-test"
  local input_format="$1"
  local input_file="$2"
  local output_format="$3"

  local test_instance="${test_name}_${input_format}_${output_format}"

  echo "############ starting test $test_instance for keyset file"
  local output_file="$TEST_TMPDIR/${test_instance}_output.${output_format}"
  $CC_KEYSET_RW_CLI $input_format $input_file $output_format $output_file

  # Use binary representation for output comparison, as JSON representation
  # seems not to be deterministic, and the order of fields varies.
  local orig_java_keyset="$TEST_TMPDIR/${test_instance}_orig_keyset_java.bin"
  local copy_cc_keyset="$TEST_TMPDIR/${test_instance}_copy_keyset_cc.bin"
  echo "### generating JSON representation of the original keyset"
  $JAVA_KEYSET_RW_CLI convert-keyset --in-format $input_format\
    --in $input_file --out-format "BINARY" > $orig_java_keyset
  echo "### generating JSON representation of the copied keyset"
  $JAVA_KEYSET_RW_CLI convert-keyset --in-format $output_format\
    --in $output_file --out-format "BINARY" > $copy_cc_keyset
  assert_files_equal $orig_java_keyset $copy_cc_keyset
}

#############################################################################
##### Run the actual tests.

### Java BINARY, C++ BINARY
generate_symmetric_key "AEAD-test-1" "AES128_CTR_HMAC_SHA256" "BINARY"
keyset_reader_writer_basic_test "BINARY" $symmetric_key_file "BINARY"

### Java BINARY, C++ JSON
generate_symmetric_key "AEAD-test-2" "AES128_CTR_HMAC_SHA256" "BINARY"
keyset_reader_writer_basic_test "BINARY" $symmetric_key_file "JSON"

### Java JSON, C++ BINARY
generate_symmetric_key "AEAD-test-3" "AES128_CTR_HMAC_SHA256" "JSON"
keyset_reader_writer_basic_test "JSON" $symmetric_key_file "BINARY"

### Java JSON, C++ JSON
generate_symmetric_key "AEAD-test-4" "AES256_CTR_HMAC_SHA256" "JSON"
keyset_reader_writer_basic_test "JSON" $symmetric_key_file "JSON"
