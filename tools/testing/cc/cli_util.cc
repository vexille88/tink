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

#include "tools/testing/cc/cli_util.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "tink/binary_keyset_reader.h"
#include "tink/binary_keyset_writer.h"
#include "tink/cleartext_keyset_handle.h"
#include "tink/keyset_reader.h"
#include "tink/keyset_writer.h"
#include "tink/json_keyset_reader.h"
#include "tink/json_keyset_writer.h"
#include "tink/config.h"
#include "tink/keyset_handle.h"
#include "tink/config/tink_config.h"
#include "tink/util/status.h"

using crypto::tink::BinaryKeysetReader;
using crypto::tink::BinaryKeysetWriter;
using crypto::tink::CleartextKeysetHandle;
using crypto::tink::Config;
using crypto::tink::JsonKeysetReader;
using crypto::tink::JsonKeysetWriter;
using crypto::tink::KeysetHandle;
using crypto::tink::KeysetReader;
using crypto::tink::KeysetWriter;
using crypto::tink::TinkConfig;

// static
std::unique_ptr<KeysetReader> CliUtil::GetBinaryKeysetReader(
    const std::string& filename) {
  std::clog << "Creating a BinaryKeysetReader...\n";
  std::unique_ptr<std::ifstream> keyset_stream(new std::ifstream());
  keyset_stream->open(filename, std::ifstream::in);
  auto keyset_reader_result = BinaryKeysetReader::New(std::move(keyset_stream));
  if (!keyset_reader_result.ok()) {
    std::clog << "Creation of the reader failed: "
              << keyset_reader_result.status().error_message() << std::endl;
    exit(1);
  }
  return std::move(keyset_reader_result.ValueOrDie());
}

// static
std::unique_ptr<KeysetReader> CliUtil::GetJsonKeysetReader(
    const std::string& filename) {
  std::clog << "Creating a JsonKeysetReader...\n";
  std::unique_ptr<std::ifstream> keyset_stream(new std::ifstream());
  keyset_stream->open(filename, std::ifstream::in);
  auto keyset_reader_result = JsonKeysetReader::New(std::move(keyset_stream));
  if (!keyset_reader_result.ok()) {
    std::clog << "Creation of the reader failed: "
              << keyset_reader_result.status().error_message() << std::endl;
    exit(1);
  }
  return std::move(keyset_reader_result.ValueOrDie());
}

// static
std::unique_ptr<KeysetWriter> CliUtil::GetBinaryKeysetWriter(
    const std::string& filename) {
  std::clog << "Creating a BinaryKeysetWriter...\n";
  std::unique_ptr<std::ofstream> keyset_stream(new std::ofstream());
  keyset_stream->open(filename, std::ofstream::out);
  auto keyset_writer_result = BinaryKeysetWriter::New(std::move(keyset_stream));
  if (!keyset_writer_result.ok()) {
    std::clog << "Creation of the writer failed: "
              << keyset_writer_result.status().error_message() << std::endl;
    exit(1);
  }
  return std::move(keyset_writer_result.ValueOrDie());
}

// static
std::unique_ptr<KeysetWriter> CliUtil::GetJsonKeysetWriter(
    const std::string& filename) {
  std::clog << "Creating a JsonKeysetWriter...\n";
  std::unique_ptr<std::ofstream> keyset_stream(new std::ofstream());
  keyset_stream->open(filename, std::ifstream::out);
  auto keyset_writer_result = JsonKeysetWriter::New(std::move(keyset_stream));
  if (!keyset_writer_result.ok()) {
    std::clog << "Creation of the writer failed: "
              << keyset_writer_result.status().error_message() << std::endl;
    exit(1);
  }
  return std::move(keyset_writer_result.ValueOrDie());
}

// static
std::unique_ptr<KeysetHandle> CliUtil::ReadKeyset(const std::string& filename) {
  auto keyset_reader = GetBinaryKeysetReader(filename);
  auto keyset_handle_result =
      CleartextKeysetHandle::Read(std::move(keyset_reader));
  if (!keyset_handle_result.ok()) {
    std::clog << "Reading the keyset failed: "
              << keyset_handle_result.status().error_message() << std::endl;
    exit(1);
  }
  return std::move(keyset_handle_result.ValueOrDie());
}

// static
void CliUtil::InitTink() {
  std::clog << "Initializing Tink...\n";
  auto status = TinkConfig::Register();
  if (!status.ok()) {
    std::clog << "Initialization of Tink failed: "
              << status.error_message() << std::endl;
    exit(1);
  }
}

// static
std::string CliUtil::Read(const std::string& filename) {
  std::clog << "Reading the input...\n";
  std::ifstream input_stream;
  input_stream.open(filename, std::ifstream::in);
  if (!input_stream.is_open()) {
    std::clog << "Error opening input file " << filename << std::endl;
    exit(1);
  }
  std::stringstream input;
  input << input_stream.rdbuf();
  input_stream.close();
  return input.str();
}

// static
void CliUtil::Write(const std::string& output,
                    const std::string& filename) {
  std::clog << "Writing the output...\n";
  std::ofstream output_stream(filename,
                              std::ofstream::out | std::ofstream::binary);
  if (!output_stream.is_open()) {
    std::clog << "Error opening output file " << filename << std::endl;
    exit(1);
  }
  output_stream << output;
  output_stream.close();
}
