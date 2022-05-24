// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "txt/platform.h"

#if defined(SK_BUILD_FONT_MGR_FOR_PREVIEW_WIN)
#include "include/core/SkFontMgr.h"
bool SkFontMgr::physicalDeviceFonts = false;
#endif

namespace txt {

#if defined(SK_BUILD_FONT_MGR_FOR_PREVIEW_WIN)
std::string GetDefaultFontFamily() {
#ifdef OHOS_STANDARD_SYSTEM
  if (SkFontMgr::physicalDeviceFonts) {
    return "HarmonyOS-Sans";
  } else {
    return "Arial";
  }
#else
  if (SkFontMgr::physicalDeviceFonts) {
    return "sans-serif";
  } else {
    return "Arial";
  }
#endif
}
#else
std::string GetDefaultFontFamily() {
  return "Arial";
}
#endif

sk_sp<SkFontMgr> GetDefaultFontManager() {
  return SkFontMgr::RefDefault();
}

FontManagerType GetDefaultFontManagerType() {
  return FontManagerType::NONE;
}

}  // namespace txt
