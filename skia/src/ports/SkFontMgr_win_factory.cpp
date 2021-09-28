/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 * 2021.9.9 SkFontMgr::Factory() on previewer of ohos.
 *           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.
 */

#ifdef SK_BUILD_FONT_MGR_FOR_PREVIEW_WIN
#include "include/core/SkFontMgr.h"
#include "src/ports/SkFontMgr_preview.h"
#include "include/ports/SkTypeface_win.h"

bool SkFontMgr::physicalDeviceFonts = false;
sk_sp<SkFontMgr> SkFontMgr::Factory()
{
    if (SkFontMgr::physicalDeviceFonts) {
        return SkFontMgr_New_Preview();
    } else {
        return SkFontMgr_New_DirectWrite();
    }
}
#endif /* SK_BUILD_FONT_MGR_FOR_PREVIEW_WIN */