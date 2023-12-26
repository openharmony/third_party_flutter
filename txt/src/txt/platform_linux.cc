// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "txt/platform.h"

#if defined(SK_BUILD_FONT_MGR_FOR_PREVIEW_LINUX)
#include "include/core/SkFontMgr.h"

#endif

#ifdef FLUTTER_USE_FONTCONFIG
#include "third_party/skia/include/ports/SkFontMgr_fontconfig.h"
#else
#include "third_party/skia/include/ports/SkFontMgr_directory.h"
#endif

namespace txt {

#if defined(SK_BUILD_FONT_MGR_FOR_PREVIEW_LINUX)
std::vector<std::string> GetDefaultFontFamilies()
{
    if (SkFontMgr::runtimeOS == "OHOS") {
        return {"HarmonyOS-Sans"};
    }
    if (SkFontMgr::runtimeOS == "OHOS_Container") {
        return {"sans-serif"};
    }
    return {"Ubuntu", "Cantarell", "DejaVu Sans", "Liberation Sans", "Arial"};;
}
#else
std::vector<std::string> GetDefaultFontFamilies() {
  return {"Ubuntu", "Cantarell", "DejaVu Sans", "Liberation Sans", "Arial"};
}
#endif

#ifndef USE_ROSEN_DRAWING
sk_sp<SkFontMgr> GetDefaultFontManager()
{
#ifdef SK_BUILD_FONT_MGR_FOR_PREVIEW_LINUX
    return SkFontMgr::RefDefault();
#elif FLUTTER_USE_FONTCONFIG
    return SkFontMgr_New_FontConfig(nullptr);
#else
  return SkFontMgr_New_Custom_Directory("/usr/share/fonts/");
#endif
}
#else
std::shared_ptr<RSFontMgr> GetDefaultFontManager()
{
    return RSFontMgr::CreateDefaultFontMgr();
}
#endif

}  // namespace txt
