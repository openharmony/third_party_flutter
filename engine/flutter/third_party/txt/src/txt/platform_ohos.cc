// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Framework adapted on ACE.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#include "txt/platform.h"

namespace txt {

std::string GetDefaultFontFamily() {
    return "HarmonyOS-Sans";
}

sk_sp<SkFontMgr> GetDefaultFontManager() {
    return SkFontMgr::RefDefault();
}

FontManagerType GetDefaultFontManagerType() {
    return FontManagerType::DEFAULT_OHOS;
}

}  // namespace txt
