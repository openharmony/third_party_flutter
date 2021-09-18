// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Framework adapted on ACE.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#include "txt/platform.h"

#ifndef OHOS_STANDARD_SYSTEM
#include "src/ports/SkFontMgr_ohos.h"
#endif

namespace txt {

std::string GetDefaultFontFamily() {
#ifdef OHOS_STANDARD_SYSTEM
    return "HarmonyOS-Sans";
#else
    return "sans-serif";
#endif
}

sk_sp<SkFontMgr> GetDefaultFontManager() {
#ifdef OHOS_STANDARD_SYSTEM
    return SkFontMgr::RefDefault();
#else
    return SkFontMgr_New_OHOS();
#endif
}

FontManagerType GetDefaultFontManagerType() {
    return FontManagerType::DEFAULT_OHOS;
}

}  // namespace txt
