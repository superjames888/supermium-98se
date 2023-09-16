// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_WIN_DIRECT_WRITE_H_
#define UI_GFX_WIN_DIRECT_WRITE_H_

#include <dwrite.h>

#include "base/strings/string_piece.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "ui/gfx/gfx_export.h"

namespace gfx {
namespace win {
	
// Returns whether DirectWrite font rendering should be used.
GFX_EXPORT bool ShouldUseDirectWrite();

GFX_EXPORT void InitializeDirectWrite();

// Returns true if we are using DirectWrite for font metrics and rendering.
GFX_EXPORT bool IsDirectWriteEnabled();

// Creates a DirectWrite factory, if using DirectWrite.
GFX_EXPORT void CreateDWriteFactory(IDWriteFactory** factory);

// Returns the global DirectWrite factory.
GFX_EXPORT IDWriteFactory* GetDirectWriteFactory();

// Retrieves the localized string for a given locale. If locale is empty,
// retrieves the first element of |names|.
GFX_EXPORT absl::optional<std::string> RetrieveLocalizedString(
    IDWriteLocalizedStrings* names,
    const std::string& locale);

// Retrieves the localized font name for a given locale. If locale is empty,
// retrieves the default native font name.
GFX_EXPORT absl::optional<std::string> RetrieveLocalizedFontName(
    base::StringPiece font_name,
    const std::string& locale);

}  // namespace win
}  // namespace gfx

#endif  // UI_GFX_WIN_DIRECT_WRITE_H_
