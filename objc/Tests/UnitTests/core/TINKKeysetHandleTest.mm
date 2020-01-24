/**
 * Copyright 2017 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 **************************************************************************
 */

#import "objc/TINKKeysetHandle.h"
#import "objc/core/TINKKeysetHandle_Internal.h"

#import <Security/Security.h>
#import <XCTest/XCTest.h>

#import "objc/TINKAead.h"
#import "objc/TINKAeadKeyTemplate.h"
#import "objc/TINKAllConfig.h"
#import "objc/TINKBinaryKeysetReader.h"
#import "objc/TINKConfig.h"
#import "objc/TINKHybridKeyTemplate.h"
#import "objc/TINKSignatureKeyTemplate.h"
#import "objc/aead/TINKAeadInternal.h"
#import "objc/util/TINKStrings.h"
#import "proto/Tink.pbobjc.h"

#include "tink/binary_keyset_reader.h"
#include "tink/util/status.h"
#include "tink/util/keyset_util.h"
#include "tink/util/test_util.h"
#include "proto/tink.pb.h"

// Variables used to hold the serialized keyset data.
static NSData *gBadSerializedKeyset;
static NSData *gGoodSerializedKeyset;

// Verbatim copy of the service constant defined in TINKKeychainKeysetReader.
static NSString *const kTinkService = @"com.google.crypto.tink";

// Keyset names used in the tests below.
static NSString *const kGoodKeysetName = @"com.google.crypto.tink.goodKeyset";
static NSString *const kBadKeysetName = @"com.google.crypto.tink.badKeyset";
static NSString *const kNonExistentKeysetName = @"com.google.crypto.tink.noSuchKeyset";

static TINKPBKeyset *gKeyset;

@interface TINKKeysetHandleTest : XCTestCase
@end

@implementation TINKKeysetHandleTest

+ (void)setUp {
  google::crypto::tink::Keyset ccKeyset;
  google::crypto::tink::Keyset::Key ccKey;

  crypto::tink::test::AddTinkKey("some key type", 42, ccKey,
                                 google::crypto::tink::KeyStatusType::ENABLED,
                                 google::crypto::tink::KeyData::SYMMETRIC, &ccKeyset);
  crypto::tink::test::AddRawKey("some other key type", 711, ccKey,
                                google::crypto::tink::KeyStatusType::ENABLED,
                                google::crypto::tink::KeyData::SYMMETRIC, &ccKeyset);
  ccKeyset.set_primary_key_id(42);

  std::string serializedKeyset = ccKeyset.SerializeAsString();
  gGoodSerializedKeyset = TINKStringToNSData(serializedKeyset);

  NSError *error = nil;
  gKeyset = [TINKPBKeyset
      parseFromData:[NSData dataWithBytes:serializedKeyset.data() length:serializedKeyset.length()]
              error:&error];
  XCTAssertNotNil(gKeyset);
  XCTAssertNil(error);

  gBadSerializedKeyset = TINKStringToNSData("some weird string");

  error = nil;
  TINKAllConfig *allConfig = [[TINKAllConfig alloc] initWithError:&error];
  XCTAssertNotNil(allConfig);
  XCTAssertNil(error);

  XCTAssertTrue([TINKConfig registerConfig:allConfig error:&error]);
  XCTAssertNil(error);
}

- (void)setUp {
  // Add the two keysets in the keychain. We do this here because we can use XCTAssert to test that
  // SecItemAdd succeeds. It would be better in +setUp but XCTAssert isn't available.
  static dispatch_once_t onceToken;
  dispatch_once(&onceToken, ^{
    NSDictionary *attr = @{
      (__bridge id)kSecClass : (__bridge id)kSecClassGenericPassword,
      (__bridge id)
      kSecAttrAccessible : (__bridge id)kSecAttrAccessibleAfterFirstUnlockThisDeviceOnly,
      (__bridge id)kSecAttrService : kTinkService,
      (__bridge id)kSecAttrSynchronizable : (__bridge id)kCFBooleanFalse,
    };
    NSMutableDictionary *attributes = [attr mutableCopy];

    // Store the keyset.
    [attributes setObject:kGoodKeysetName forKey:(__bridge id)kSecAttrAccount];
    [attributes setObject:gGoodSerializedKeyset forKey:(__bridge id)kSecValueData];
    OSStatus status = SecItemAdd((__bridge CFDictionaryRef)attributes, NULL);
    XCTAssertTrue(status == errSecSuccess || status == errSecDuplicateItem);

    // Store the bad keyset.
    [attributes setObject:kBadKeysetName forKey:(__bridge id)kSecAttrAccount];
    [attributes setObject:gBadSerializedKeyset forKey:(__bridge id)kSecValueData];
    status = SecItemAdd((__bridge CFDictionaryRef)attributes, NULL);
    XCTAssertTrue(status == errSecSuccess || status == errSecDuplicateItem);
  });
}

- (void)testGoodEncryptedKeyset_Binary {
  auto ccAead =
      std::unique_ptr<crypto::tink::Aead>(new crypto::tink::test::DummyAead("dummy aead 42"));
  TINKAeadInternal *aead = [[TINKAeadInternal alloc] initWithCCAead:std::move(ccAead)];

  NSData *keysetCiphertext = [aead encrypt:gKeyset.data withAdditionalData:[NSData data] error:nil];

  XCTAssertNotNil(keysetCiphertext);

  TINKPBEncryptedKeyset *encryptedKeyset = [[TINKPBEncryptedKeyset alloc] init];
  encryptedKeyset.encryptedKeyset = keysetCiphertext;

  TINKBinaryKeysetReader *reader =
      [[TINKBinaryKeysetReader alloc] initWithSerializedKeyset:encryptedKeyset.data error:nil];

  TINKKeysetHandle *handle =
      [[TINKKeysetHandle alloc] initWithKeysetReader:reader andKey:aead error:nil];
  XCTAssertNotNil(handle);
  std::string output;
  crypto::tink::KeysetUtil::GetKeyset(*handle.ccKeysetHandle).SerializeToString(&output);

  XCTAssertTrue(
      [gKeyset.data isEqualToData:[NSData dataWithBytes:output.data() length:output.size()]]);
}

- (void)testWrongAead_Binary {
  auto ccAead =
      std::unique_ptr<crypto::tink::Aead>(new crypto::tink::test::DummyAead("dummy aead 42"));
  TINKAeadInternal *aead = [[TINKAeadInternal alloc] initWithCCAead:std::move(ccAead)];

  NSData *keysetCiphertext = [aead encrypt:gKeyset.data withAdditionalData:[NSData data] error:nil];

  TINKPBEncryptedKeyset *encryptedKeyset = [[TINKPBEncryptedKeyset alloc] init];
  encryptedKeyset.encryptedKeyset = keysetCiphertext;

  TINKBinaryKeysetReader *reader =
      [[TINKBinaryKeysetReader alloc] initWithSerializedKeyset:encryptedKeyset.data error:nil];

  auto ccWrongAead =
      std::unique_ptr<crypto::tink::Aead>(new crypto::tink::test::DummyAead("wrong aead"));
  TINKAeadInternal *wrongAead = [[TINKAeadInternal alloc] initWithCCAead:std::move(ccWrongAead)];

  NSError *error = nil;
  TINKKeysetHandle *handle =
      [[TINKKeysetHandle alloc] initWithKeysetReader:reader andKey:wrongAead error:&error];
  XCTAssertNil(handle);
  XCTAssertEqual(error.code, crypto::tink::util::error::INVALID_ARGUMENT);
}

- (void)testNoKeysetInCiphertext_Binary {
  auto ccAead =
      std::unique_ptr<crypto::tink::Aead>(new crypto::tink::test::DummyAead("dummy aead 42"));
  TINKAeadInternal *aead = [[TINKAeadInternal alloc] initWithCCAead:std::move(ccAead)];
  NSData *keysetCiphertext =
      [aead encrypt:[@"not a serialized keyset" dataUsingEncoding:NSUTF8StringEncoding]
          withAdditionalData:[NSData data]
                       error:nil];

  TINKBinaryKeysetReader *reader =
      [[TINKBinaryKeysetReader alloc] initWithSerializedKeyset:keysetCiphertext error:nil];

  NSError *error = nil;
  TINKKeysetHandle *handle =
      [[TINKKeysetHandle alloc] initWithKeysetReader:reader andKey:aead error:&error];
  XCTAssertNil(handle);
  XCTAssertEqual(error.code, crypto::tink::util::error::INVALID_ARGUMENT);
}

- (void)testWrongCiphertext_Binary {
  auto ccAead =
      std::unique_ptr<crypto::tink::Aead>(new crypto::tink::test::DummyAead("dummy aead 42"));
  TINKAeadInternal *aead = [[TINKAeadInternal alloc] initWithCCAead:std::move(ccAead)];
  NSString *keysetCiphertext = @"totally wrong ciphertext";

  TINKPBEncryptedKeyset *encryptedKeyset = [[TINKPBEncryptedKeyset alloc] init];
  encryptedKeyset.encryptedKeyset = [keysetCiphertext dataUsingEncoding:NSUTF8StringEncoding];

  TINKBinaryKeysetReader *reader =
      [[TINKBinaryKeysetReader alloc] initWithSerializedKeyset:encryptedKeyset.data error:nil];
  NSError *error = nil;
  TINKKeysetHandle *handle =
      [[TINKKeysetHandle alloc] initWithKeysetReader:reader andKey:aead error:&error];
  XCTAssertNil(handle);
  XCTAssertEqual(error.code, crypto::tink::util::error::INVALID_ARGUMENT);
}

- (void)testValidKeyTemplate {
  NSError *error = nil;
  TINKHybridKeyTemplate *keyTemplate =
      [[TINKHybridKeyTemplate alloc] initWithKeyTemplate:TINKEciesP256HkdfHmacSha256Aes128Gcm
                                                   error:&error];
  XCTAssertNotNil(keyTemplate);
  XCTAssertNil(error);

  // TODO(candrian): Update this test as now C++ does support key templates.
  TINKKeysetHandle *handle =
      [[TINKKeysetHandle alloc] initWithKeyTemplate:keyTemplate error:&error];
  XCTAssertNotNil(handle);
  XCTAssertNil(error);
}

- (void)testReuseKeysetReader {
  auto ccAead =
      std::unique_ptr<crypto::tink::Aead>(new crypto::tink::test::DummyAead("dummy aead 42"));
  TINKAeadInternal *aead = [[TINKAeadInternal alloc] initWithCCAead:std::move(ccAead)];

  NSData *keysetCiphertext = [aead encrypt:gKeyset.data withAdditionalData:[NSData data] error:nil];

  XCTAssertNotNil(keysetCiphertext);

  TINKPBEncryptedKeyset *encryptedKeyset = [[TINKPBEncryptedKeyset alloc] init];
  encryptedKeyset.encryptedKeyset = keysetCiphertext;

  TINKBinaryKeysetReader *reader =
      [[TINKBinaryKeysetReader alloc] initWithSerializedKeyset:encryptedKeyset.data error:nil];

  TINKKeysetHandle *handle =
      [[TINKKeysetHandle alloc] initWithKeysetReader:reader andKey:aead error:nil];
  XCTAssertNotNil(handle);

  NSError *error = nil;
  XCTAssertNil([[TINKKeysetHandle alloc] initWithKeysetReader:reader andKey:aead error:&error]);
  XCTAssertNotNil(error);
  XCTAssertEqual(error.code, crypto::tink::util::error::RESOURCE_EXHAUSTED);
  XCTAssertTrue(
      [error.localizedFailureReason containsString:@"A KeysetReader can be used only once."]);
}

- (void)testGoodKeysetFromKeychain {
  NSError *error = nil;
  TINKKeysetHandle *handle =
      [[TINKKeysetHandle alloc] initFromKeychainWithName:kGoodKeysetName error:&error];
  XCTAssertNotNil(handle);
  XCTAssertNil(error);

  // Verify the contents of the keyset.
  auto ccKeyset = crypto::tink::KeysetUtil::GetKeyset(*handle.ccKeysetHandle);
  std::string serializedCCKeyset;
  XCTAssertTrue(ccKeyset.SerializeToString(&serializedCCKeyset));
  XCTAssertTrue(
      [gGoodSerializedKeyset isEqualToData:[NSData dataWithBytes:serializedCCKeyset.data()
                                                          length:serializedCCKeyset.length()]]);
}

- (void)testBadKeysetFromKeychain {
  NSError *error = nil;
  TINKKeysetHandle *handle =
      [[TINKKeysetHandle alloc] initFromKeychainWithName:kBadKeysetName error:&error];
  XCTAssertNil(handle);
  XCTAssertNotNil(error);
  XCTAssertEqual(error.code, crypto::tink::util::error::INVALID_ARGUMENT);
  XCTAssertTrue([error.localizedFailureReason
      containsString:@"Could not parse the input stream as a Keyset-proto."]);
}

- (void)testUnknownKeysetFromKeychain {
  NSError *error = nil;
  TINKKeysetHandle *handle =
      [[TINKKeysetHandle alloc] initFromKeychainWithName:kNonExistentKeysetName error:&error];
  XCTAssertNil(handle);
  XCTAssertNotNil(error);
  XCTAssertEqual(error.code, crypto::tink::util::error::NOT_FOUND);
  XCTAssertTrue([error.localizedFailureReason
      containsString:@"A keyset with the given name wasn't found in the keychain."]);
}

- (void)testWriteKeysetToKeychain {
  static NSString *const kKeysetName = @"com.google.crypto.tink.randomaeadkeyset";

  NSError *error = nil;
  // Generate a new fresh keyset for Aead.
  TINKAeadKeyTemplate *tpl =
      [[TINKAeadKeyTemplate alloc] initWithKeyTemplate:TINKAes128Gcm error:&error];
  XCTAssertNotNil(tpl);
  XCTAssertNil(error);

  TINKKeysetHandle *handle1 = [[TINKKeysetHandle alloc] initWithKeyTemplate:tpl error:&error];
  XCTAssertNotNil(handle1);
  XCTAssertNil(error);

  // Delete any previous keychain items with the same name.
  NSDictionary *attr = @{
    (__bridge id)kSecClass : (__bridge id)kSecClassGenericPassword,
    (__bridge id)kSecAttrAccessible : (__bridge id)kSecAttrAccessibleAfterFirstUnlockThisDeviceOnly,
    (__bridge id)kSecAttrService : kTinkService,
    (__bridge id)kSecAttrSynchronizable : (__bridge id)kCFBooleanFalse,
    (__bridge id)kSecAttrAccount : kKeysetName,
  };
  OSStatus deleteStatus = SecItemDelete((__bridge CFDictionaryRef)attr);
  XCTAssertTrue(deleteStatus == errSecSuccess || deleteStatus == errSecItemNotFound);

  // Store the keyset in the iOS keychain.
  XCTAssertTrue([handle1 writeToKeychainWithName:kKeysetName overwrite:NO error:&error]);
  XCTAssertNil(error);

  // Generate a new handle using the stored keyset.
  TINKKeysetHandle *handle2 =
      [[TINKKeysetHandle alloc] initFromKeychainWithName:kKeysetName error:&error];
  XCTAssertNotNil(handle2);
  XCTAssertNil(error);

  // Compare the two keysets, verify that they are identical.
  auto keyset1 = crypto::tink::KeysetUtil::GetKeyset(*handle1.ccKeysetHandle);
  std::string serializedKeyset1;
  XCTAssertTrue(keyset1.SerializeToString(&serializedKeyset1));

  auto keyset2 = crypto::tink::KeysetUtil::GetKeyset(*handle2.ccKeysetHandle);
  std::string serializedKeyset2;
  XCTAssertTrue(keyset2.SerializeToString(&serializedKeyset2));

  XCTAssertTrue(serializedKeyset1 == serializedKeyset2);
}

- (void)testDeleteKeysetFromKeychain {
  static NSString *const kKeysetName = @"com.google.crypto.tink.somekeyset";

  NSError *error = nil;
  // Generate a new fresh keyset for Aead.
  TINKAeadKeyTemplate *tpl =
      [[TINKAeadKeyTemplate alloc] initWithKeyTemplate:TINKAes128Gcm error:&error];
  XCTAssertNotNil(tpl);
  XCTAssertNil(error);

  TINKKeysetHandle *handle1 = [[TINKKeysetHandle alloc] initWithKeyTemplate:tpl error:&error];
  XCTAssertNotNil(handle1);
  XCTAssertNil(error);

  // Store the keyset in the iOS keychain.
  XCTAssertTrue([handle1 writeToKeychainWithName:kKeysetName overwrite:NO error:&error]);
  XCTAssertNil(error);

  // Delete it.
  XCTAssertTrue([TINKKeysetHandle deleteFromKeychainWithName:kKeysetName error:&error]);
  XCTAssertNil(error);

  // Try again. Should succeed with ItemNotFound.
  XCTAssertTrue([TINKKeysetHandle deleteFromKeychainWithName:kKeysetName error:&error]);
  XCTAssertNil(error);
}

- (void)testPublicKeysetHandleWithHandle {
  NSError *error = nil;
  TINKSignatureKeyTemplate *tpl =
      [[TINKSignatureKeyTemplate alloc] initWithKeyTemplate:TINKEcdsaP256 error:&error];
  XCTAssertNotNil(tpl);
  XCTAssertNil(error);

  TINKKeysetHandle *handle = [[TINKKeysetHandle alloc] initWithKeyTemplate:tpl error:&error];
  XCTAssertNotNil(handle);
  XCTAssertNil(error);

  TINKKeysetHandle *publicHandle = [TINKKeysetHandle publicKeysetHandleWithHandle:handle
                                                                            error:&error];
  XCTAssertNotNil(publicHandle);
  XCTAssertNil(error);

  auto keyset = crypto::tink::KeysetUtil::GetKeyset(*handle.ccKeysetHandle);
  auto public_keyset = crypto::tink::KeysetUtil::GetKeyset(*publicHandle.ccKeysetHandle);
  XCTAssertEqual(keyset.primary_key_id(), public_keyset.primary_key_id());
  XCTAssertEqual(keyset.key_size(), public_keyset.key_size());
  XCTAssertEqual(keyset.key(0).status(), public_keyset.key(0).status());
  XCTAssertEqual(keyset.key(0).key_id(), public_keyset.key(0).key_id());
  XCTAssertEqual(keyset.key(0).output_prefix_type(), public_keyset.key(0).output_prefix_type());
  XCTAssertEqual(google::crypto::tink::KeyData::ASYMMETRIC_PUBLIC,
                 public_keyset.key(0).key_data().key_material_type());
}

- (void)testPublicKeysetHandleWithHandleFailedNotAsymmetric {
  NSError *error = nil;
  TINKAeadKeyTemplate *tpl = [[TINKAeadKeyTemplate alloc] initWithKeyTemplate:TINKAes128Eax
                                                                        error:&error];
  XCTAssertNotNil(tpl);
  XCTAssertNil(error);

  TINKKeysetHandle *handle = [[TINKKeysetHandle alloc] initWithKeyTemplate:tpl error:&error];
  XCTAssertNotNil(handle);
  XCTAssertNil(error);

  TINKKeysetHandle *publicHandle = [TINKKeysetHandle publicKeysetHandleWithHandle:handle
                                                                            error:&error];
  XCTAssertNil(publicHandle);
  XCTAssertNotNil(error);
  XCTAssertTrue(error.code == crypto::tink::util::error::INVALID_ARGUMENT);
  XCTAssertTrue([error.localizedFailureReason
      containsString:@"Key material is not of type KeyData::ASYMMETRIC_PRIVATE"]);
}

@end

