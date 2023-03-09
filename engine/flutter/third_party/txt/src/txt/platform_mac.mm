// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <TargetConditionals.h>
#include "flutter/fml/platform/darwin/platform_version.h"
#include "txt/platform.h"

#if TARGET_OS_EMBEDDED || TARGET_OS_SIMULATOR
#include <UIKit/UIKit.h>
#define FONT_CLASS UIFont
#else  // TARGET_OS_EMBEDDED
#include <AppKit/AppKit.h>
#define FONT_CLASS NSFont
#endif  // TARGET_OS_EMBEDDED

#if defined(SK_BUILD_FONT_MGR_FOR_PREVIEW_MAC)
#include "include/core/SkFontMgr.h"
#endif

namespace txt {

#if defined(SK_BUILD_FONT_MGR_FOR_PREVIEW_MAC)
std::string GetDefaultFontFamily() {
  if (SkFontMgr::runtimeOS == "OHOS") {
    return "HarmonyOS-Sans";
  }
  if (SkFontMgr::runtimeOS == "OHOS_Container") {
    return "sans-serif";
  }
  if (fml::IsPlatformVersionAtLeast(9)) {
    return [FONT_CLASS systemFontOfSize:14].familyName.UTF8String;
  } else {
    return "Helvetica";
  }
}

#else
std::string GetDefaultFontFamily() {
  if (fml::IsPlatformVersionAtLeast(9)) {
    return [FONT_CLASS systemFontOfSize:14].familyName.UTF8String;
  } else {
    return "Helvetica";
  }
}
#endif

sk_sp<SkFontMgr> GetDefaultFontManager() {
  return SkFontMgr::RefDefault();
}

}  // namespace txt
