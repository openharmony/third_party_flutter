// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_PAINTING_RRECT_H_
#define FLUTTER_LIB_UI_PAINTING_RRECT_H_

#include "third_party/skia/include/core/SkRRect.h"

namespace flutter {

class RRect {
 public:
  SkRRect sk_rrect;
  bool is_null;
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_PAINTING_RRECT_H_
