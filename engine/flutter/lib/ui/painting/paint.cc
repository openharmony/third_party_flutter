// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/paint.h"

#include "flutter/fml/logging.h"
#include "flutter/lib/ui/painting/color_filter.h"
#include "flutter/lib/ui/painting/image_filter.h"
#include "flutter/lib/ui/painting/shader.h"
#include "third_party/skia/include/core/SkColorFilter.h"
#include "third_party/skia/include/core/SkImageFilter.h"
#include "third_party/skia/include/core/SkMaskFilter.h"
#include "third_party/skia/include/core/SkShader.h"
#include "third_party/skia/include/core/SkString.h"

namespace flutter {

// Must be kept in sync with the default in painting.dart.
constexpr uint32_t kColorDefault = 0xFF000000;

// Must be kept in sync with the MaskFilter private constants in painting.dart.
enum MaskFilterType { Null, Blur };

Paint::Paint(uint32_t encoded_color) {
  if (encoded_color) {
    SkColor color = encoded_color ^ kColorDefault;
    paint_.setColor(color);
  }
}

Paint::Paint(Dart_Handle paint_objects, Dart_Handle paint_data) {
}

} // namespace flutter

