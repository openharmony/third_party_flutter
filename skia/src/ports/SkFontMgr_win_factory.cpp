/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 * 2021.9.9 SkFontMgr::Factory() on previewer of ohos.
 *           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.
 * 2022.9.4 SkFontMgr::Factory() on previewer of ohos and container.
 *           Copyright (c) 2022 Huawei Device Co., Ltd. All rights reserved.
 */

#include "include/core/SkFontMgr.h"
#include "include/ports/SkTypeface_win.h"
#include "src/ports/SkFontMgr_preview.h"

std::string SkFontMgr::runtimeOS = "OHOS";
SK_API sk_sp<SkFontMgr> SkFontMgr_New_OHOS(const char *path);

sk_sp<SkFontMgr> SkFontMgr::Factory()
{
    if (SkFontMgr::runtimeOS == "OHOS") {
        return SkFontMgr_New_OHOS(nullptr);
    }
    if (SkFontMgr::runtimeOS == "OHOS_Container") {
        return SkFontMgr_New_Preview();
    }
    return SkFontMgr_New_DirectWrite();
}