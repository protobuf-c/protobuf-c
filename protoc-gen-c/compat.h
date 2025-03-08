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

#ifndef PROTOBUF_C_PROTOC_GEN_C_COMPAT_H__
#define PROTOBUF_C_PROTOC_GEN_C_COMPAT_H__

#if GOOGLE_PROTOBUF_VERSION >= 4022000
# define GOOGLE_ARRAYSIZE	ABSL_ARRAYSIZE
# define GOOGLE_CHECK_EQ	ABSL_CHECK_EQ
# define GOOGLE_DCHECK_GE	ABSL_DCHECK_GE
# define GOOGLE_LOG		ABSL_LOG
#endif

#if GOOGLE_PROTOBUF_VERSION >= 6030000
# include <absl/strings/string_view.h>
#else
# include <string>
#endif

namespace protobuf_c {

namespace compat {

#if GOOGLE_PROTOBUF_VERSION >= 6030000
typedef absl::string_view StringView;
#else
typedef const std::string& StringView;
#endif

}  // namespace compat

}  // namespace protobuf_c

#endif  // PROTOBUF_C_PROTOC_GEN_C_COMPAT_H__
