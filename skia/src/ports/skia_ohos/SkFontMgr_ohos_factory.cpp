// Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if SK_BUILD_FONT_MGR_FOR_PREVIEW_WIN || SK_BUILD_FONT_MGR_FOR_OHOS
#include "SkFontMgr.h"
SK_API sk_sp<SkFontMgr> SkFontMgr_New_OHOS(const char* path);
#endif

#if defined(SK_BUILD_FONT_MGR_FOR_OHOS)
/*! To implement the porting layer to return the default factory for Harmony platform
 * \return the default font manager for Harmony platform
 */
sk_sp<SkFontMgr> SkFontMgr::Factory()
{
    return SkFontMgr_New_OHOS(nullptr);
}
#endif /* SK_BUILD_FONT_MGR_FOR_OHOS */

#if defined(SK_BUILD_FONT_MGR_FOR_PREVIEW_WIN)
#include "include/ports/SkTypeface_win.h"

sk_sp<SkFontMgr> SkFontMgr::Factory()
{
    if (SkFontMgr::physicalDeviceFonts) {
        return SkFontMgr_New_OHOS(nullptr);
    } else {
        return SkFontMgr_New_DirectWrite();
    }
}
#endif /* SK_BUILD_FONT_MGR_FOR_PREVIEW_WIN */