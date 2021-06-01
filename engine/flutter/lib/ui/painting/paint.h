// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_PAINTING_PAINT_H_
#define FLUTTER_LIB_UI_PAINTING_PAINT_H_

#include "flutter/lib/ui/dart_wrapper.h"
#include "third_party/skia/include/core/SkPaint.h"

namespace flutter {

class Paint {
 public:
  Paint() = default;
  Paint(Dart_Handle paint_objects, Dart_Handle paint_data);
  Paint(uint32_t encoded_color);

  const SkPaint* paint() const { return is_null_ ? nullptr : &paint_; }

  SkPaint* paint() { return is_null_ ? nullptr : &paint_; }
 private:

  SkPaint paint_;
  bool is_null_ = false;
};

// The PaintData argument is a placeholder to receive encoded data for Paint
// objects. The data is actually processed by DartConverter<Paint>, which reads
// both at the given index and at the next index (which it assumes is a byte
// data for a Paint object).
class PaintData {};

}  // namespace blink

#endif  // FLUTTER_LIB_UI_PAINTING_PAINT_H_
