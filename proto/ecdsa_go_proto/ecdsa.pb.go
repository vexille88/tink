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
// source: proto/ecdsa.proto

/*
Package ecdsa_proto is a generated protocol buffer package.

It is generated from these files:
	proto/ecdsa.proto

It has these top-level messages:
	EcdsaParams
	EcdsaPublicKey
	EcdsaPrivateKey
	EcdsaKeyFormat
*/
package ecdsa_go_proto

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"
import google_crypto_tink "github.com/google/tink/proto/common_go_proto"

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// This is a compile-time assertion to ensure that this generated file
// is compatible with the proto package it is being compiled against.
// A compilation error at this line likely means your copy of the
// proto package needs to be updated.
const _ = proto.ProtoPackageIsVersion2 // please upgrade the proto package

type EcdsaSignatureEncoding int32

const (
	EcdsaSignatureEncoding_UNKNOWN_ENCODING EcdsaSignatureEncoding = 0
	// The signature's format is r || s, where r and s are zero-padded and have the same size in
	// bytes as the order of the curve. For example, for NIST P-256 curve, r and s are zero-padded to
	// 32 bytes.
	EcdsaSignatureEncoding_IEEE_P1363 EcdsaSignatureEncoding = 1
	// The signature is encoded using ASN.1
	// (https://tools.ietf.org/html/rfc5480#appendix-A):
	// ECDSA-Sig-Value :: = SEQUENCE {
	//  r INTEGER,
	//  s INTEGER
	// }
	EcdsaSignatureEncoding_DER EcdsaSignatureEncoding = 2
)

var EcdsaSignatureEncoding_name = map[int32]string{
	0: "UNKNOWN_ENCODING",
	1: "IEEE_P1363",
	2: "DER",
}
var EcdsaSignatureEncoding_value = map[string]int32{
	"UNKNOWN_ENCODING": 0,
	"IEEE_P1363":       1,
	"DER":              2,
}

func (x EcdsaSignatureEncoding) String() string {
	return proto.EnumName(EcdsaSignatureEncoding_name, int32(x))
}
func (EcdsaSignatureEncoding) EnumDescriptor() ([]byte, []int) { return fileDescriptor0, []int{0} }

// Protos for Ecdsa.
type EcdsaParams struct {
	// Required.
	HashType google_crypto_tink.HashType `protobuf:"varint,1,opt,name=hash_type,json=hashType,enum=google.crypto.tink.HashType" json:"hash_type,omitempty"`
	// Required.
	Curve google_crypto_tink.EllipticCurveType `protobuf:"varint,2,opt,name=curve,enum=google.crypto.tink.EllipticCurveType" json:"curve,omitempty"`
	// Required.
	Encoding EcdsaSignatureEncoding `protobuf:"varint,3,opt,name=encoding,enum=google.crypto.tink.EcdsaSignatureEncoding" json:"encoding,omitempty"`
}

func (m *EcdsaParams) Reset()                    { *m = EcdsaParams{} }
func (m *EcdsaParams) String() string            { return proto.CompactTextString(m) }
func (*EcdsaParams) ProtoMessage()               {}
func (*EcdsaParams) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{0} }

func (m *EcdsaParams) GetHashType() google_crypto_tink.HashType {
	if m != nil {
		return m.HashType
	}
	return google_crypto_tink.HashType_UNKNOWN_HASH
}

func (m *EcdsaParams) GetCurve() google_crypto_tink.EllipticCurveType {
	if m != nil {
		return m.Curve
	}
	return google_crypto_tink.EllipticCurveType_UNKNOWN_CURVE
}

func (m *EcdsaParams) GetEncoding() EcdsaSignatureEncoding {
	if m != nil {
		return m.Encoding
	}
	return EcdsaSignatureEncoding_UNKNOWN_ENCODING
}

// key_type: type.googleapis.com/google.crypto.tink.EcdsaPublicKey
type EcdsaPublicKey struct {
	// Required.
	Version uint32 `protobuf:"varint,1,opt,name=version" json:"version,omitempty"`
	// Required.
	Params *EcdsaParams `protobuf:"bytes,2,opt,name=params" json:"params,omitempty"`
	// Affine coordinates of the public key in bigendian representation. The
	// public key is a point (x, y) on the curve defined by params.curve. For
	// ECDH, it is crucial to verify whether the public key point (x, y) is on the
	// private's key curve. For ECDSA, such verification is a defense in depth.
	// Required.
	X []byte `protobuf:"bytes,3,opt,name=x,proto3" json:"x,omitempty"`
	// Required.
	Y []byte `protobuf:"bytes,4,opt,name=y,proto3" json:"y,omitempty"`
}

func (m *EcdsaPublicKey) Reset()                    { *m = EcdsaPublicKey{} }
func (m *EcdsaPublicKey) String() string            { return proto.CompactTextString(m) }
func (*EcdsaPublicKey) ProtoMessage()               {}
func (*EcdsaPublicKey) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{1} }

func (m *EcdsaPublicKey) GetVersion() uint32 {
	if m != nil {
		return m.Version
	}
	return 0
}

func (m *EcdsaPublicKey) GetParams() *EcdsaParams {
	if m != nil {
		return m.Params
	}
	return nil
}

func (m *EcdsaPublicKey) GetX() []byte {
	if m != nil {
		return m.X
	}
	return nil
}

func (m *EcdsaPublicKey) GetY() []byte {
	if m != nil {
		return m.Y
	}
	return nil
}

// key_type: type.googleapis.com/google.crypto.tink.EcdsaPrivateKey
type EcdsaPrivateKey struct {
	// Required.
	Version uint32 `protobuf:"varint,1,opt,name=version" json:"version,omitempty"`
	// Required.
	PublicKey *EcdsaPublicKey `protobuf:"bytes,2,opt,name=public_key,json=publicKey" json:"public_key,omitempty"`
	// Unsigned big integer in bigendian representation.
	// Required.
	KeyValue []byte `protobuf:"bytes,3,opt,name=key_value,json=keyValue,proto3" json:"key_value,omitempty"`
}

func (m *EcdsaPrivateKey) Reset()                    { *m = EcdsaPrivateKey{} }
func (m *EcdsaPrivateKey) String() string            { return proto.CompactTextString(m) }
func (*EcdsaPrivateKey) ProtoMessage()               {}
func (*EcdsaPrivateKey) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{2} }

func (m *EcdsaPrivateKey) GetVersion() uint32 {
	if m != nil {
		return m.Version
	}
	return 0
}

func (m *EcdsaPrivateKey) GetPublicKey() *EcdsaPublicKey {
	if m != nil {
		return m.PublicKey
	}
	return nil
}

func (m *EcdsaPrivateKey) GetKeyValue() []byte {
	if m != nil {
		return m.KeyValue
	}
	return nil
}

type EcdsaKeyFormat struct {
	// Required.
	Params *EcdsaParams `protobuf:"bytes,2,opt,name=params" json:"params,omitempty"`
}

func (m *EcdsaKeyFormat) Reset()                    { *m = EcdsaKeyFormat{} }
func (m *EcdsaKeyFormat) String() string            { return proto.CompactTextString(m) }
func (*EcdsaKeyFormat) ProtoMessage()               {}
func (*EcdsaKeyFormat) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{3} }

func (m *EcdsaKeyFormat) GetParams() *EcdsaParams {
	if m != nil {
		return m.Params
	}
	return nil
}

func init() {
	proto.RegisterType((*EcdsaParams)(nil), "google.crypto.tink.EcdsaParams")
	proto.RegisterType((*EcdsaPublicKey)(nil), "google.crypto.tink.EcdsaPublicKey")
	proto.RegisterType((*EcdsaPrivateKey)(nil), "google.crypto.tink.EcdsaPrivateKey")
	proto.RegisterType((*EcdsaKeyFormat)(nil), "google.crypto.tink.EcdsaKeyFormat")
	proto.RegisterEnum("google.crypto.tink.EcdsaSignatureEncoding", EcdsaSignatureEncoding_name, EcdsaSignatureEncoding_value)
}

func init() { proto.RegisterFile("proto/ecdsa.proto", fileDescriptor0) }

var fileDescriptor0 = []byte{
	// 426 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0x9c, 0x92, 0xd1, 0x6a, 0xd4, 0x40,
	0x14, 0x86, 0x9d, 0xad, 0x6e, 0x77, 0x4f, 0xeb, 0xba, 0x0e, 0x22, 0x41, 0x0b, 0x4a, 0x40, 0x28,
	0xbd, 0xc8, 0x62, 0x17, 0x14, 0xf1, 0xca, 0xb6, 0xd3, 0xba, 0x2c, 0xa4, 0x4b, 0x5a, 0x15, 0xbc,
	0x09, 0xb3, 0xd3, 0x21, 0x19, 0x36, 0xc9, 0x0c, 0x93, 0xc9, 0xd2, 0xb9, 0xf2, 0x01, 0x7c, 0x0b,
	0xdf, 0xc7, 0x77, 0x92, 0xcc, 0xa4, 0x45, 0x70, 0xeb, 0x45, 0xef, 0xce, 0x4f, 0xfe, 0x2f, 0xf3,
	0xff, 0x87, 0x03, 0x4f, 0x95, 0x96, 0x46, 0x4e, 0x38, 0xbb, 0xaa, 0x69, 0xe4, 0x66, 0x8c, 0x33,
	0x29, 0xb3, 0x82, 0x47, 0x4c, 0x5b, 0x65, 0x64, 0x64, 0x44, 0xb5, 0x7a, 0x81, 0xbd, 0x8d, 0xc9,
	0xb2, 0x94, 0x95, 0xf7, 0x85, 0xbf, 0x11, 0xec, 0x90, 0x96, 0x5b, 0x50, 0x4d, 0xcb, 0x1a, 0x7f,
	0x80, 0x61, 0x4e, 0xeb, 0x3c, 0x35, 0x56, 0xf1, 0x00, 0xbd, 0x46, 0xfb, 0xa3, 0xc3, 0xbd, 0xe8,
	0xdf, 0x7f, 0x45, 0x9f, 0x69, 0x9d, 0x5f, 0x5a, 0xc5, 0x93, 0x41, 0xde, 0x4d, 0xf8, 0x23, 0x3c,
	0x62, 0x8d, 0x5e, 0xf3, 0xa0, 0xe7, 0xb0, 0x37, 0x9b, 0x30, 0x52, 0x14, 0x42, 0x19, 0xc1, 0x8e,
	0x5b, 0xa3, 0xe3, 0x3d, 0x83, 0x4f, 0x61, 0xc0, 0x2b, 0x26, 0xaf, 0x44, 0x95, 0x05, 0x5b, 0x8e,
	0x3f, 0xd8, 0xc8, 0xb7, 0x51, 0x2f, 0x44, 0x56, 0x51, 0xd3, 0x68, 0x4e, 0x3a, 0x22, 0xb9, 0x65,
	0xc3, 0x1f, 0x30, 0xf2, 0x75, 0x9a, 0x65, 0x21, 0xd8, 0x9c, 0x5b, 0x1c, 0xc0, 0xf6, 0x9a, 0xeb,
	0x5a, 0xc8, 0xca, 0xf5, 0x79, 0x9c, 0xdc, 0x48, 0xfc, 0x1e, 0xfa, 0xca, 0xb5, 0x76, 0x89, 0x77,
	0x0e, 0x5f, 0xdd, 0xf9, 0xa2, 0x5f, 0x4e, 0xd2, 0xd9, 0xf1, 0x2e, 0xa0, 0x6b, 0x97, 0x72, 0x37,
	0x41, 0xd7, 0xad, 0xb2, 0xc1, 0x43, 0xaf, 0x6c, 0xf8, 0x13, 0xc1, 0x13, 0xcf, 0x68, 0xb1, 0xa6,
	0x86, 0xff, 0x3f, 0xc2, 0x27, 0x00, 0xe5, 0x92, 0xa6, 0x2b, 0x6e, 0xbb, 0x18, 0xe1, 0xdd, 0x31,
	0x6e, 0x4a, 0x25, 0x43, 0x75, 0xdb, 0xef, 0x25, 0x0c, 0x57, 0xdc, 0xa6, 0x6b, 0x5a, 0x34, 0xbc,
	0x0b, 0x35, 0x58, 0x71, 0xfb, 0xb5, 0xd5, 0xe1, 0xac, 0x5b, 0xc7, 0x9c, 0xdb, 0x53, 0xa9, 0x4b,
	0x6a, 0xee, 0x5d, 0xfa, 0xe0, 0x0c, 0x9e, 0x6f, 0xde, 0x3e, 0x7e, 0x06, 0xe3, 0x2f, 0xf1, 0x3c,
	0x3e, 0xff, 0x16, 0xa7, 0x24, 0x3e, 0x3e, 0x3f, 0x99, 0xc5, 0x67, 0xe3, 0x07, 0x78, 0x04, 0x30,
	0x23, 0x84, 0xa4, 0x8b, 0xb7, 0xd3, 0x77, 0xd3, 0x31, 0xc2, 0xdb, 0xb0, 0x75, 0x42, 0x92, 0x71,
	0xef, 0xe8, 0x02, 0xf6, 0x98, 0x2c, 0x37, 0x3d, 0xeb, 0x4e, 0x72, 0x81, 0xbe, 0xef, 0x67, 0xc2,
	0xe4, 0xcd, 0x32, 0x62, 0xb2, 0x9c, 0x78, 0xdb, 0xa4, 0xfd, 0x3e, 0xf9, 0xeb, 0xcc, 0x53, 0x37,
	0xff, 0xea, 0xf5, 0x2f, 0x67, 0xf1, 0x7c, 0x71, 0xb4, 0xec, 0x3b, 0x3d, 0xfd, 0x13, 0x00, 0x00,
	0xff, 0xff, 0xd2, 0x43, 0xbe, 0xe7, 0x0b, 0x03, 0x00, 0x00,
}
