// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/rand_util.h"

#include <windows.h>

#include <stddef.h>
#include <stdint.h>

// #define needed to link in RtlGenRandom(), a.k.a. SystemFunction036.  See the
// "Community Additions" comment on MSDN here:
// http://msdn.microsoft.com/en-us/library/windows/desktop/aa387694.aspx
#define SystemFunction036 NTAPI SystemFunction036
#include <NTSecAPI.h>
#undef SystemFunction036

#include <algorithm>
#include <atomic>
#include <limits>

#include "base/check.h"
#include "base/feature_list.h"
#include "third_party/boringssl/src/include/openssl/crypto.h"
#include "third_party/boringssl/src/include/openssl/rand.h"

namespace base {

namespace internal {

namespace {

// The BoringSSl helpers are duplicated in rand_util_fuchsia.cc and
// rand_util_posix.cc.
std::atomic<bool> g_use_boringssl;

BASE_FEATURE(kUseBoringSSLForRandBytes,
             "UseBoringSSLForRandBytes",
             FEATURE_DISABLED_BY_DEFAULT);

}  // namespace

void ConfigureBoringSSLBackedRandBytesFieldTrial() {
  g_use_boringssl.store(FeatureList::IsEnabled(kUseBoringSSLForRandBytes),
                        std::memory_order_relaxed);
}

bool UseBoringSSLForRandBytes() {
  return g_use_boringssl.load(std::memory_order_relaxed);
}

}  // namespace internal

namespace {

void RandBytes(span<uint8_t> output, bool avoid_allocation) {
  if (!avoid_allocation && internal::UseBoringSSLForRandBytes()) {
    // Ensure BoringSSL is initialized so it can use things like RDRAND.
    CRYPTO_library_init();
    // BoringSSL's RAND_bytes always returns 1. Any error aborts the program.
    (void)RAND_bytes(output.data(), output.size());
    return;
  }

  unsigned char* output_ptr = output.data();
  size_t output_size = output.size();
  while (output_size > 0) {
    const ULONG output_bytes_this_pass = static_cast<ULONG>(std::min(
        output_size, static_cast<size_t>(std::numeric_limits<ULONG>::max())));
    const bool success =
        RtlGenRandom(output_ptr, output_bytes_this_pass) != FALSE;
    CHECK(success);
    output_size -= output_bytes_this_pass;
    output_ptr += output_bytes_this_pass;
}

}

}  // namespace

void RandBytes(span<uint8_t> output) {
  RandBytes(output, /*avoid_allocation=*/false);
}

void RandBytes(void* output, size_t output_length) {
  RandBytes(make_span(reinterpret_cast<uint8_t*>(output), output_length),
            /*avoid_allocation=*/false);
}

namespace internal {

double RandDoubleAvoidAllocation() {
  uint64_t number;
  RandBytes(as_writable_bytes(make_span(&number, 1u)),
            /*avoid_allocation=*/true);
  // This transformation is explained in rand_util.cc.
  return (number >> 11) * 0x1.0p-53;
}

}  // namespace internal

}  // namespace base
