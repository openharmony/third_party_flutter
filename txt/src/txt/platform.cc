// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "txt/platform.h"

#if defined(SK_BUILD_FONT_MGR_FOR_PREVIEW_WIN)
#include "include/core/SkFontMgr.h"
#endif

namespace txt {

#if defined(SK_BUILD_FONT_MGR_FOR_PREVIEW_WIN)
std::vector<std::string> GetDefaultFontFamilies() {
    if (SkFontMgr::runtimeOS == "OHOS") {
        return {"HarmonyOS-Sans"};
    }
    if (SkFontMgr::runtimeOS == "OHOS_Container") {
        return {"sans-serif"};
    }
    return {"Arial"};
}
#else
std::vector<std::string> GetDefaultFontFamilies() {
    return {"Arial"};
}
#endif

#ifndef USE_ROSEN_DRAWING
sk_sp<SkFontMgr> GetDefaultFontManager() {
    return SkFontMgr::RefDefault();
}
#else
std::shared_ptr<RSFontMgr> GetDefaultFontManager() {
    return RSFontMgr::CreateDefaultFontMgr();
}
#endif

FontManagerType GetDefaultFontManagerType() {
    return FontManagerType::DEFAULT_OHOS;
}

}  // namespace txt
