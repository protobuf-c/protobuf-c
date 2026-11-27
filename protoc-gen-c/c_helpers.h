// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// http://code.google.com/p/protobuf/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Author: kenton@google.com (Kenton Varda)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.

// Copyright (c) 2008-2025, Dave Benson and the protobuf-c authors.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Modified to implement C code by Dave Benson.

#ifndef PROTOBUF_C_PROTOC_GEN_C_C_HELPERS_H__
#define PROTOBUF_C_PROTOC_GEN_C_C_HELPERS_H__

#include <string>
#include <vector>
#include <sstream>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>

#include <protobuf-c/protobuf-c.pb.h>

#include "compat.h"

namespace protobuf_c {

// --- Borrowed from stubs. ---
template <typename T> std::string SimpleItoa(T n) {
  std::stringstream stream;
  stream << n;
  return stream.str();
}

std::string SimpleFtoa(float f);
std::string SimpleDtoa(double f);
void SplitStringUsing(compat::StringView str, const char *delim, std::vector<std::string> *out);
std::string CEscape(compat::StringView src);
inline bool HasSuffixString(compat::StringView str, compat::StringView suffix) { return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0; }
inline std::string StripSuffixString(compat::StringView str, compat::StringView suffix) { if (HasSuffixString(str, suffix)) { return std::string(str.substr(0, str.size() - suffix.size())); } else { return std::string(str); } }
char* FastHexToBuffer(int i, char* buffer);

// Get the (unqualified) name that should be used for this field in C code.
// The name is coerced to lower-case to emulate proto1 behavior.  People
// should be using lowercase-with-underscores style for proto field names
// anyway, so normally this just returns field->name().
std::string FieldName(const google::protobuf::FieldDescriptor* field);

// Get macro string for deprecated field
std::string FieldDeprecated(const google::protobuf::FieldDescriptor* field);

// Returns the scope where the field was defined (for extensions, this is
// different from the message type to which the field applies).
inline const google::protobuf::Descriptor* FieldScope(const google::protobuf::FieldDescriptor* field) {
  return field->is_extension() ?
    field->extension_scope() : field->containing_type();
}

// convert a CamelCase class name into an all uppercase affair
// with underscores separating words, e.g. MyClass becomes MY_CLASS.
std::string CamelToUpper(compat::StringView class_name);
std::string CamelToLower(compat::StringView class_name);

// lowercased, underscored name to camel case
std::string ToCamel(compat::StringView name);

// lowercase the string
std::string ToLower(compat::StringView class_name);
std::string ToUpper(compat::StringView class_name);

// full_name() to lowercase with underscores
std::string FullNameToLower(compat::StringView full_name, const google::protobuf::FileDescriptor *file);
std::string FullNameToUpper(compat::StringView full_name, const google::protobuf::FileDescriptor *file);

// full_name() to c-typename (with underscores for packages, otherwise camel case)
std::string FullNameToC(compat::StringView class_name, const google::protobuf::FileDescriptor *file);

// Splits, indents, formats, and prints comment lines
void PrintComment(google::protobuf::io::Printer* printer, std::string comment);

// make a string of spaces as long as input
std::string ConvertToSpaces(compat::StringView input);

// Strips ".proto" or ".protodevel" from the end of a filename.
std::string StripProto(compat::StringView filename);

// Get the C++ type name for a primitive type (e.g. "double", "::google::protobuf::int32", etc.).
// Note:  non-built-in type names will be qualified, meaning they will start
// with a ::.  If you are using the type as a template parameter, you will
// need to insure there is a space between the < and the ::, because the
// ridiculous C++ standard defines "<:" to be a synonym for "[".
const char* PrimitiveTypeName(google::protobuf::FieldDescriptor::CppType type);

// Get the declared type name in CamelCase format, as is used e.g. for the
// methods of WireFormat.  For example, TYPE_INT32 becomes "Int32".
const char* DeclaredTypeMethodName(google::protobuf::FieldDescriptor::Type type);

// Convert a file name into a valid identifier.
std::string FilenameIdentifier(compat::StringView filename);

// return 'required', 'optional', or 'repeated'
std::string GetLabelName(google::protobuf::FieldDescriptor::Label label);

// write IntRanges entries for a bunch of sorted values.
// returns the number of ranges there are to bsearch.
unsigned WriteIntRanges(google::protobuf::io::Printer* printer, int n_values, const int *values, compat::StringView name);

struct NameIndex
{
  unsigned index;
  compat::StringView name;
};
int compare_name_indices_by_name(const void*, const void*);

// Returns true if the field has "implicit" presence: a singular field
// without a has_ flag, whose zero value is simply not serialized.
//
// proto2 fields (and editions fields with features.field_presence = EXPLICIT
// or LEGACY_REQUIRED) have explicit presence.  Every proto3 field is treated
// as implicit here, as protobuf-c always has (including proto3 "optional"
// and oneof members); editions files follow features.field_presence.
inline bool FieldHasImplicitPresence(const google::protobuf::FieldDescriptor* field) {
  auto proto = google::protobuf::FileDescriptorProto();
  field->file()->CopyTo(&proto);

  if (proto.has_syntax()) {
    const std::string& syntax = proto.syntax();
    if (syntax == "proto3")
      return true;
    if (syntax == "editions")
      return !field->has_presence();
  }
  return false;
}

// The resolved edition features of a descriptor, as emitted into the
// ProtobufCFeatures structure (the numeric values are those of FeatureSet in
// descriptor.proto).
struct FeatureValues {
  int field_presence;
  int enum_type;
  int repeated_field_encoding;
  int utf8_validation;
  int message_encoding;
  int json_format;
};

// Returns the C expression ("&name" or "NULL") that points to the
// ProtobufCFeatures for the given descriptor.  The definitions of the
// referenced objects are written by PrintFeaturesDefinitions().
std::string FeaturesReference(const google::protobuf::Descriptor* desc);
std::string FeaturesReference(const google::protobuf::FieldDescriptor* desc);
std::string FeaturesReference(const google::protobuf::EnumDescriptor* desc);

// Writes a static ProtobufCFeatures for each distinct set of features used by
// the messages, fields and enums of the file.
void PrintFeaturesDefinitions(const google::protobuf::FileDescriptor* file,
                              google::protobuf::io::Printer* printer);

}  // namespace protobuf_c

#endif  // PROTOBUF_C_PROTOC_GEN_C_C_HELPERS_H__
