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
std::string GetDefaultFontFamily()
{
    if (SkFontMgr::runtimeOS == "OHOS") {
        return "HarmonyOS-Sans";
    }
    if (SkFontMgr::runtimeOS == "OHOS_Container") {
        return "sans-serif";
    }
    return "Arial";
}
#else
std::string GetDefaultFontFamily()
{
    return "Arial";
}
#endif

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

}  // namespace txt
