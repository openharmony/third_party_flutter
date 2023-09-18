// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TXT_PLATFORM_H_
#define TXT_PLATFORM_H_

#include <string>
#include <vector>

#include "flutter/fml/macros.h"
#ifndef USE_ROSEN_DRAWING
#include "third_party/skia/include/core/SkFontMgr.h"
#else
#include "drawing.h"
#endif

namespace txt {

enum FontManagerType {
  DYNAMIC = 0,
  ASSET = 1,
  TEST = 2,
  DEFAULT_OHOS = 3,
  DEFAULT_ANDROID = 4,
  NONE = 5,
};

std::vector<std::string> GetDefaultFontFamilies();

#ifndef USE_ROSEN_DRAWING
sk_sp<SkFontMgr> GetDefaultFontManager();
#else
std::shared_ptr<RSFontMgr> GetDefaultFontManager();
#endif

FontManagerType GetDefaultFontManagerType();

}  // namespace txt

#endif  // TXT_PLATFORM_H_
