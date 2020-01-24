// Copyright 2017 Google Inc.

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//      http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
////////////////////////////////////////////////////////////////////////////////

// Code generated by protoc-gen-go. DO NOT EDIT.
// source: tink.proto

/*
Package tink_proto is a generated protocol buffer package.

It is generated from these files:
	tink.proto

It has these top-level messages:
	KeyTemplate
	KeyData
	Keyset
	KeysetInfo
	EncryptedKeyset
*/
package tink_go_proto

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// This is a compile-time assertion to ensure that this generated file
// is compatible with the proto package it is being compiled against.
// A compilation error at this line likely means your copy of the
// proto package needs to be updated.
const _ = proto.ProtoPackageIsVersion2 // please upgrade the proto package

type KeyStatusType int32

const (
	KeyStatusType_UNKNOWN_STATUS KeyStatusType = 0
	KeyStatusType_ENABLED        KeyStatusType = 1
	KeyStatusType_DISABLED       KeyStatusType = 2
	KeyStatusType_DESTROYED      KeyStatusType = 3
)

var KeyStatusType_name = map[int32]string{
	0: "UNKNOWN_STATUS",
	1: "ENABLED",
	2: "DISABLED",
	3: "DESTROYED",
}
var KeyStatusType_value = map[string]int32{
	"UNKNOWN_STATUS": 0,
	"ENABLED":        1,
	"DISABLED":       2,
	"DESTROYED":      3,
}

func (x KeyStatusType) String() string {
	return proto.EnumName(KeyStatusType_name, int32(x))
}
func (KeyStatusType) EnumDescriptor() ([]byte, []int) { return fileDescriptor0, []int{0} }

// Tink produces and accepts ciphertexts or signatures that consist
// of a prefix and a payload. The payload and its format is determined
// entirely by the primitive, but the prefix has to be one of the following
// 4 types:
//   - Legacy: prefix is 5 bytes, starts with \x00 and followed by a 4-byte
//             key id that is computed from the key material.
//   - Crunchy: prefix is 5 bytes, starts with \x00 and followed by a 4-byte
//             key id that is generated randomly.
//   - Tink  : prefix is 5 bytes, starts with \x01 and followed by 4-byte
//             key id that is generated randomly.
//   - Raw   : prefix is 0 byte, i.e., empty.
type OutputPrefixType int32

const (
	OutputPrefixType_UNKNOWN_PREFIX OutputPrefixType = 0
	OutputPrefixType_TINK           OutputPrefixType = 1
	OutputPrefixType_LEGACY         OutputPrefixType = 2
	OutputPrefixType_RAW            OutputPrefixType = 3
	// CRUNCHY is like LEGACY, but with two differences:
	//   - Its key id is generated randomly (like TINK)
	//   - Its signature schemes don't append zero to sign messages
	OutputPrefixType_CRUNCHY OutputPrefixType = 4
)

var OutputPrefixType_name = map[int32]string{
	0: "UNKNOWN_PREFIX",
	1: "TINK",
	2: "LEGACY",
	3: "RAW",
	4: "CRUNCHY",
}
var OutputPrefixType_value = map[string]int32{
	"UNKNOWN_PREFIX": 0,
	"TINK":           1,
	"LEGACY":         2,
	"RAW":            3,
	"CRUNCHY":        4,
}

func (x OutputPrefixType) String() string {
	return proto.EnumName(OutputPrefixType_name, int32(x))
}
func (OutputPrefixType) EnumDescriptor() ([]byte, []int) { return fileDescriptor0, []int{1} }

type KeyData_KeyMaterialType int32

const (
	KeyData_UNKNOWN_KEYMATERIAL KeyData_KeyMaterialType = 0
	KeyData_SYMMETRIC           KeyData_KeyMaterialType = 1
	KeyData_ASYMMETRIC_PRIVATE  KeyData_KeyMaterialType = 2
	KeyData_ASYMMETRIC_PUBLIC   KeyData_KeyMaterialType = 3
	KeyData_REMOTE              KeyData_KeyMaterialType = 4
)

var KeyData_KeyMaterialType_name = map[int32]string{
	0: "UNKNOWN_KEYMATERIAL",
	1: "SYMMETRIC",
	2: "ASYMMETRIC_PRIVATE",
	3: "ASYMMETRIC_PUBLIC",
	4: "REMOTE",
}
var KeyData_KeyMaterialType_value = map[string]int32{
	"UNKNOWN_KEYMATERIAL": 0,
	"SYMMETRIC":           1,
	"ASYMMETRIC_PRIVATE":  2,
	"ASYMMETRIC_PUBLIC":   3,
	"REMOTE":              4,
}

func (x KeyData_KeyMaterialType) String() string {
	return proto.EnumName(KeyData_KeyMaterialType_name, int32(x))
}
func (KeyData_KeyMaterialType) EnumDescriptor() ([]byte, []int) { return fileDescriptor0, []int{1, 0} }

type KeyTemplate struct {
	// Required.
	TypeUrl string `protobuf:"bytes,1,opt,name=type_url,json=typeUrl" json:"type_url,omitempty"`
	// Optional.
	// If missing, it means the key type doesn't require a *KeyFormat proto.
	Value []byte `protobuf:"bytes,2,opt,name=value,proto3" json:"value,omitempty"`
	// Optional.
	// If missing, uses OutputPrefixType.TINK.
	OutputPrefixType OutputPrefixType `protobuf:"varint,3,opt,name=output_prefix_type,json=outputPrefixType,enum=google.crypto.tink.OutputPrefixType" json:"output_prefix_type,omitempty"`
}

func (m *KeyTemplate) Reset()                    { *m = KeyTemplate{} }
func (m *KeyTemplate) String() string            { return proto.CompactTextString(m) }
func (*KeyTemplate) ProtoMessage()               {}
func (*KeyTemplate) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{0} }

func (m *KeyTemplate) GetTypeUrl() string {
	if m != nil {
		return m.TypeUrl
	}
	return ""
}

func (m *KeyTemplate) GetValue() []byte {
	if m != nil {
		return m.Value
	}
	return nil
}

func (m *KeyTemplate) GetOutputPrefixType() OutputPrefixType {
	if m != nil {
		return m.OutputPrefixType
	}
	return OutputPrefixType_UNKNOWN_PREFIX
}

// The actual *Key-proto is wrapped in a KeyData message, which in addition
// to this serialized proto contains also type_url identifying the
// definition of *Key-proto (as in KeyFormat-message), and some extra metadata
// about the type key material.
type KeyData struct {
	// Required.
	TypeUrl string `protobuf:"bytes,1,opt,name=type_url,json=typeUrl" json:"type_url,omitempty"`
	// Required.
	Value []byte `protobuf:"bytes,2,opt,name=value,proto3" json:"value,omitempty"`
	// Required.
	KeyMaterialType KeyData_KeyMaterialType `protobuf:"varint,3,opt,name=key_material_type,json=keyMaterialType,enum=google.crypto.tink.KeyData_KeyMaterialType" json:"key_material_type,omitempty"`
}

func (m *KeyData) Reset()                    { *m = KeyData{} }
func (m *KeyData) String() string            { return proto.CompactTextString(m) }
func (*KeyData) ProtoMessage()               {}
func (*KeyData) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{1} }

func (m *KeyData) GetTypeUrl() string {
	if m != nil {
		return m.TypeUrl
	}
	return ""
}

func (m *KeyData) GetValue() []byte {
	if m != nil {
		return m.Value
	}
	return nil
}

func (m *KeyData) GetKeyMaterialType() KeyData_KeyMaterialType {
	if m != nil {
		return m.KeyMaterialType
	}
	return KeyData_UNKNOWN_KEYMATERIAL
}

// A Tink user works usually not with single keys, but with keysets,
// to enable key rotation.  The keys in a keyset can belong to different
// implementations/key types, but must all implement the same primitive.
// Any given keyset (and any given key) can be used for one primitive only.
type Keyset struct {
	// Identifies key used to generate new crypto data (encrypt, sign).
	// Required.
	PrimaryKeyId uint32 `protobuf:"varint,1,opt,name=primary_key_id,json=primaryKeyId" json:"primary_key_id,omitempty"`
	// Actual keys in the Keyset.
	// Required.
	Key []*Keyset_Key `protobuf:"bytes,2,rep,name=key" json:"key,omitempty"`
}

func (m *Keyset) Reset()                    { *m = Keyset{} }
func (m *Keyset) String() string            { return proto.CompactTextString(m) }
func (*Keyset) ProtoMessage()               {}
func (*Keyset) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{2} }

func (m *Keyset) GetPrimaryKeyId() uint32 {
	if m != nil {
		return m.PrimaryKeyId
	}
	return 0
}

func (m *Keyset) GetKey() []*Keyset_Key {
	if m != nil {
		return m.Key
	}
	return nil
}

type Keyset_Key struct {
	// Contains the actual, instantiation specific key proto.
	// By convention, each key proto contains a version field.
	KeyData *KeyData      `protobuf:"bytes,1,opt,name=key_data,json=keyData" json:"key_data,omitempty"`
	Status  KeyStatusType `protobuf:"varint,2,opt,name=status,enum=google.crypto.tink.KeyStatusType" json:"status,omitempty"`
	// Identifies a key within a keyset, is a part of metadata
	// of a ciphertext/signature.
	KeyId uint32 `protobuf:"varint,3,opt,name=key_id,json=keyId" json:"key_id,omitempty"`
	// Determines the prefix of the ciphertexts/signatures produced by this key.
	// This value is copied verbatim from the key template.
	OutputPrefixType OutputPrefixType `protobuf:"varint,4,opt,name=output_prefix_type,json=outputPrefixType,enum=google.crypto.tink.OutputPrefixType" json:"output_prefix_type,omitempty"`
}

func (m *Keyset_Key) Reset()                    { *m = Keyset_Key{} }
func (m *Keyset_Key) String() string            { return proto.CompactTextString(m) }
func (*Keyset_Key) ProtoMessage()               {}
func (*Keyset_Key) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{2, 0} }

func (m *Keyset_Key) GetKeyData() *KeyData {
	if m != nil {
		return m.KeyData
	}
	return nil
}

func (m *Keyset_Key) GetStatus() KeyStatusType {
	if m != nil {
		return m.Status
	}
	return KeyStatusType_UNKNOWN_STATUS
}

func (m *Keyset_Key) GetKeyId() uint32 {
	if m != nil {
		return m.KeyId
	}
	return 0
}

func (m *Keyset_Key) GetOutputPrefixType() OutputPrefixType {
	if m != nil {
		return m.OutputPrefixType
	}
	return OutputPrefixType_UNKNOWN_PREFIX
}

// Represents a "safe" Keyset that doesn't contain any actual key material,
// thus can be used for logging or monitoring. Most fields are copied from
// Keyset.
type KeysetInfo struct {
	// See Keyset.primary_key_id.
	PrimaryKeyId uint32 `protobuf:"varint,1,opt,name=primary_key_id,json=primaryKeyId" json:"primary_key_id,omitempty"`
	// KeyInfos in the KeysetInfo.
	// Each KeyInfo is corresponding to a Key in the corresponding Keyset.
	KeyInfo []*KeysetInfo_KeyInfo `protobuf:"bytes,2,rep,name=key_info,json=keyInfo" json:"key_info,omitempty"`
}

func (m *KeysetInfo) Reset()                    { *m = KeysetInfo{} }
func (m *KeysetInfo) String() string            { return proto.CompactTextString(m) }
func (*KeysetInfo) ProtoMessage()               {}
func (*KeysetInfo) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{3} }

func (m *KeysetInfo) GetPrimaryKeyId() uint32 {
	if m != nil {
		return m.PrimaryKeyId
	}
	return 0
}

func (m *KeysetInfo) GetKeyInfo() []*KeysetInfo_KeyInfo {
	if m != nil {
		return m.KeyInfo
	}
	return nil
}

type KeysetInfo_KeyInfo struct {
	// the type url of this key,
	// e.g., type.googleapis.com/google.crypto.tink.HmacKey.
	TypeUrl string `protobuf:"bytes,1,opt,name=type_url,json=typeUrl" json:"type_url,omitempty"`
	// See Keyset.Key.status.
	Status KeyStatusType `protobuf:"varint,2,opt,name=status,enum=google.crypto.tink.KeyStatusType" json:"status,omitempty"`
	// See Keyset.Key.key_id.
	KeyId uint32 `protobuf:"varint,3,opt,name=key_id,json=keyId" json:"key_id,omitempty"`
	// See Keyset.Key.output_prefix_type.
	OutputPrefixType OutputPrefixType `protobuf:"varint,4,opt,name=output_prefix_type,json=outputPrefixType,enum=google.crypto.tink.OutputPrefixType" json:"output_prefix_type,omitempty"`
}

func (m *KeysetInfo_KeyInfo) Reset()                    { *m = KeysetInfo_KeyInfo{} }
func (m *KeysetInfo_KeyInfo) String() string            { return proto.CompactTextString(m) }
func (*KeysetInfo_KeyInfo) ProtoMessage()               {}
func (*KeysetInfo_KeyInfo) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{3, 0} }

func (m *KeysetInfo_KeyInfo) GetTypeUrl() string {
	if m != nil {
		return m.TypeUrl
	}
	return ""
}

func (m *KeysetInfo_KeyInfo) GetStatus() KeyStatusType {
	if m != nil {
		return m.Status
	}
	return KeyStatusType_UNKNOWN_STATUS
}

func (m *KeysetInfo_KeyInfo) GetKeyId() uint32 {
	if m != nil {
		return m.KeyId
	}
	return 0
}

func (m *KeysetInfo_KeyInfo) GetOutputPrefixType() OutputPrefixType {
	if m != nil {
		return m.OutputPrefixType
	}
	return OutputPrefixType_UNKNOWN_PREFIX
}

// Represents a keyset that is encrypted with a master key.
type EncryptedKeyset struct {
	// Required.
	EncryptedKeyset []byte `protobuf:"bytes,2,opt,name=encrypted_keyset,json=encryptedKeyset,proto3" json:"encrypted_keyset,omitempty"`
	// Optional.
	KeysetInfo *KeysetInfo `protobuf:"bytes,3,opt,name=keyset_info,json=keysetInfo" json:"keyset_info,omitempty"`
}

func (m *EncryptedKeyset) Reset()                    { *m = EncryptedKeyset{} }
func (m *EncryptedKeyset) String() string            { return proto.CompactTextString(m) }
func (*EncryptedKeyset) ProtoMessage()               {}
func (*EncryptedKeyset) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{4} }

func (m *EncryptedKeyset) GetEncryptedKeyset() []byte {
	if m != nil {
		return m.EncryptedKeyset
	}
	return nil
}

func (m *EncryptedKeyset) GetKeysetInfo() *KeysetInfo {
	if m != nil {
		return m.KeysetInfo
	}
	return nil
}

func init() {
	proto.RegisterType((*KeyTemplate)(nil), "google.crypto.tink.KeyTemplate")
	proto.RegisterType((*KeyData)(nil), "google.crypto.tink.KeyData")
	proto.RegisterType((*Keyset)(nil), "google.crypto.tink.Keyset")
	proto.RegisterType((*Keyset_Key)(nil), "google.crypto.tink.Keyset.Key")
	proto.RegisterType((*KeysetInfo)(nil), "google.crypto.tink.KeysetInfo")
	proto.RegisterType((*KeysetInfo_KeyInfo)(nil), "google.crypto.tink.KeysetInfo.KeyInfo")
	proto.RegisterType((*EncryptedKeyset)(nil), "google.crypto.tink.EncryptedKeyset")
	proto.RegisterEnum("google.crypto.tink.KeyStatusType", KeyStatusType_name, KeyStatusType_value)
	proto.RegisterEnum("google.crypto.tink.OutputPrefixType", OutputPrefixType_name, OutputPrefixType_value)
	proto.RegisterEnum("google.crypto.tink.KeyData_KeyMaterialType", KeyData_KeyMaterialType_name, KeyData_KeyMaterialType_value)
}

func init() { proto.RegisterFile("tink.proto", fileDescriptor0) }

var fileDescriptor0 = []byte{
	// 653 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0xd4, 0x54, 0xcd, 0x6e, 0xd3, 0x40,
	0x10, 0xae, 0xed, 0x34, 0x49, 0x27, 0x6d, 0xb2, 0x5d, 0x28, 0x84, 0x82, 0xaa, 0x10, 0x55, 0x10,
	0x8a, 0x94, 0xa2, 0x20, 0x21, 0x71, 0x42, 0x4e, 0xb2, 0x80, 0xe5, 0xfc, 0x69, 0xe3, 0x50, 0xc2,
	0xc5, 0x72, 0x9b, 0x6d, 0xb1, 0x9c, 0xc4, 0x96, 0xb3, 0x46, 0xf8, 0xc0, 0x03, 0x70, 0xe5, 0x11,
	0x78, 0x14, 0x0e, 0x1c, 0x78, 0x0d, 0x5e, 0x04, 0xed, 0xda, 0xad, 0xda, 0xd0, 0x56, 0x20, 0x4e,
	0x9c, 0x76, 0x66, 0xf6, 0xdb, 0x99, 0xf9, 0x3e, 0xcf, 0x18, 0x80, 0xbb, 0x73, 0xaf, 0x1e, 0x84,
	0x3e, 0xf7, 0x31, 0x3e, 0xf1, 0xfd, 0x93, 0x29, 0xab, 0x1f, 0x85, 0x71, 0xc0, 0xfd, 0xba, 0xb8,
	0xa9, 0x7e, 0x51, 0xa0, 0x60, 0xb2, 0xd8, 0x62, 0xb3, 0x60, 0xea, 0x70, 0x86, 0xef, 0x40, 0x9e,
	0xc7, 0x01, 0xb3, 0xa3, 0x70, 0x5a, 0x56, 0x2a, 0x4a, 0x6d, 0x8d, 0xe6, 0x84, 0x3f, 0x0a, 0xa7,
	0xf8, 0x26, 0xac, 0x7e, 0x70, 0xa6, 0x11, 0x2b, 0xab, 0x15, 0xa5, 0xb6, 0x4e, 0x13, 0x07, 0x53,
	0xc0, 0x7e, 0xc4, 0x83, 0x88, 0xdb, 0x41, 0xc8, 0x8e, 0xdd, 0x8f, 0xb6, 0x80, 0x97, 0xb5, 0x8a,
	0x52, 0x2b, 0x36, 0x76, 0xeb, 0xbf, 0x57, 0xac, 0xf7, 0x25, 0x7a, 0x20, 0xc1, 0x56, 0x1c, 0x30,
	0x8a, 0xfc, 0xa5, 0x48, 0xf5, 0xb3, 0x0a, 0x39, 0x93, 0xc5, 0x6d, 0x87, 0x3b, 0x7f, 0xdf, 0xd0,
	0x01, 0x6c, 0x7a, 0x2c, 0xb6, 0x67, 0x0e, 0x67, 0xa1, 0xeb, 0x4c, 0xcf, 0xf7, 0xf3, 0xf8, 0xb2,
	0x7e, 0xd2, 0x42, 0xe2, 0xec, 0xa6, 0x6f, 0x64, 0x5b, 0x25, 0xef, 0x62, 0xa0, 0xca, 0xa1, 0xb4,
	0x84, 0xc1, 0xb7, 0xe1, 0xc6, 0xa8, 0x67, 0xf6, 0xfa, 0x07, 0x3d, 0xdb, 0x24, 0xe3, 0xae, 0x6e,
	0x11, 0x6a, 0xe8, 0x1d, 0xb4, 0x82, 0x37, 0x60, 0x6d, 0x38, 0xee, 0x76, 0x89, 0x45, 0x8d, 0x16,
	0x52, 0xf0, 0x2d, 0xc0, 0xfa, 0x99, 0x6f, 0x0f, 0xa8, 0xf1, 0x46, 0xb7, 0x08, 0x52, 0xf1, 0x16,
	0x6c, 0x9e, 0x8f, 0x8f, 0x9a, 0x1d, 0xa3, 0x85, 0x34, 0x0c, 0x90, 0xa5, 0xa4, 0xdb, 0xb7, 0x08,
	0xca, 0x54, 0xbf, 0xab, 0x90, 0x35, 0x59, 0xbc, 0x60, 0x1c, 0xef, 0x42, 0x31, 0x08, 0xdd, 0x99,
	0x13, 0xc6, 0xb6, 0x60, 0xe8, 0x4e, 0xa4, 0x20, 0x1b, 0x74, 0x3d, 0x8d, 0x9a, 0x2c, 0x36, 0x26,
	0xf8, 0x09, 0x68, 0x1e, 0x8b, 0xcb, 0x6a, 0x45, 0xab, 0x15, 0x1a, 0x3b, 0x57, 0x30, 0x5e, 0x30,
	0x2e, 0x0e, 0x2a, 0xa0, 0xdb, 0x3f, 0x15, 0xd0, 0x4c, 0x16, 0xe3, 0x67, 0x90, 0x17, 0x79, 0x27,
	0x0e, 0x77, 0x64, 0xe6, 0x42, 0xe3, 0xee, 0x35, 0x82, 0xd1, 0x9c, 0x97, 0x7e, 0xa2, 0xe7, 0x90,
	0x5d, 0x70, 0x87, 0x47, 0x0b, 0xf9, 0x21, 0x8a, 0x8d, 0xfb, 0x57, 0xbc, 0x1a, 0x4a, 0x90, 0x14,
	0x37, 0x7d, 0x80, 0xb7, 0x20, 0x9b, 0x52, 0xd1, 0x24, 0x95, 0x55, 0x4f, 0x72, 0xb8, 0x7c, 0xa8,
	0x32, 0xff, 0x34, 0x54, 0xdf, 0x54, 0x80, 0x84, 0xb9, 0x31, 0x3f, 0xf6, 0xff, 0x50, 0x4c, 0x3d,
	0x91, 0xc4, 0x9d, 0x1f, 0xfb, 0xa9, 0xa2, 0x0f, 0xae, 0x56, 0x54, 0xe4, 0x15, 0xa6, 0x38, 0xa5,
	0x3a, 0xc2, 0xd8, 0xfe, 0xa1, 0xc8, 0x61, 0x96, 0x45, 0xaf, 0x19, 0xe6, 0xff, 0x43, 0xc4, 0x4f,
	0x50, 0x22, 0x73, 0xf9, 0x86, 0x4d, 0xd2, 0xa9, 0x7c, 0x04, 0x88, 0x9d, 0x86, 0x84, 0x94, 0x0b,
	0xc6, 0xd3, 0x85, 0x2c, 0xb1, 0x25, 0xe8, 0x0b, 0x28, 0x24, 0x80, 0x44, 0x50, 0x4d, 0xce, 0xd8,
	0xce, 0xf5, 0x82, 0x52, 0xf0, 0xce, 0xec, 0xbd, 0x2e, 0x6c, 0x5c, 0x90, 0x00, 0x63, 0x28, 0x9e,
	0x2e, 0xe0, 0xd0, 0xd2, 0xad, 0xd1, 0x10, 0xad, 0xe0, 0x02, 0xe4, 0x48, 0x4f, 0x6f, 0x76, 0x48,
	0x1b, 0x29, 0x78, 0x1d, 0xf2, 0x6d, 0x63, 0x98, 0x78, 0xaa, 0x58, 0xcb, 0x36, 0x19, 0x5a, 0xb4,
	0x3f, 0x26, 0x6d, 0xa4, 0xed, 0x51, 0x40, 0xcb, 0x9c, 0xcf, 0x67, 0x1c, 0x50, 0xf2, 0xd2, 0x78,
	0x8b, 0x56, 0x70, 0x1e, 0x32, 0x96, 0xd1, 0x33, 0x91, 0x22, 0x36, 0xb3, 0x43, 0x5e, 0xe9, 0xad,
	0x31, 0x52, 0x71, 0x0e, 0x34, 0xaa, 0x1f, 0x20, 0x4d, 0x14, 0x6c, 0xd1, 0x51, 0xaf, 0xf5, 0x7a,
	0x8c, 0x32, 0x4d, 0x0a, 0xf7, 0x8e, 0xfc, 0xd9, 0x65, 0x9c, 0xe4, 0x4f, 0x78, 0xa0, 0xbc, 0x7b,
	0x78, 0xe2, 0xf2, 0xf7, 0xd1, 0x61, 0xfd, 0xc8, 0x9f, 0xed, 0x27, 0xb0, 0x7d, 0x71, 0xbf, 0x2f,
	0xef, 0xa5, 0x69, 0x4b, 0xf3, 0xab, 0x9a, 0x15, 0x55, 0x07, 0xcd, 0xc3, 0xac, 0xf4, 0x9f, 0xfe,
	0x0a, 0x00, 0x00, 0xff, 0xff, 0x56, 0xaa, 0x4f, 0x00, 0xcd, 0x05, 0x00, 0x00,
}
