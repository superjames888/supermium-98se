// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sandbox/policy/win/sandbox_warmup.h"

#include "base/feature_list.h"
#include "sandbox/policy/features.h"

#include <windows.h>

// Note: do not copy this to add new uses of RtlGenRandom.
// Prefer: crypto::RandBytes, base::RandBytes or bcryptprimitives!ProcessPrng.
// #define needed to link in RtlGenRandom(), a.k.a. SystemFunction036.  See the
// "Community Additions" comment on MSDN here:
// http://msdn.microsoft.com/en-us/library/windows/desktop/aa387694.aspx
#define SystemFunction036 NTAPI SystemFunction036
#include <NTSecAPI.h>
#undef SystemFunction036

namespace sandbox::policy {

void WarmupRandomnessInfrastructure() {
  BYTE data[1];
    // This loads advapi!SystemFunction036 which is forwarded to
    // cryptbase!SystemFunction036. This allows boringsll and Chrome to call
    // RtlGenRandom from within the sandbox. This has the unfortunate side
    // effect of opening a handle to \\Device\KsecDD which we will later close
    // in processes that do not need this.
    RtlGenRandom(data, sizeof(data));
}

}  // namespace sandbox::policy
