/*
 * Copyright 2018 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIB_TXT_SRC_TEXT_SHADOW_H_
#define LIB_TXT_SRC_TEXT_SHADOW_H_

#ifndef USE_ROSEN_DRAWING
#include "third_party/skia/include/core/SkColor.h"
#include "third_party/skia/include/core/SkPoint.h"
#else
#include "drawing.h"
#endif

namespace txt {

class TextShadow {
 public:
#ifndef USE_ROSEN_DRAWING
  SkColor color = SK_ColorBLACK;
  SkPoint offset;
#else
  uint32_t color = RSColor::COLOR_BLACK;
  RSPoint offset;
#endif
  double blur_sigma = 0.0;

  TextShadow();

#ifndef USE_ROSEN_DRAWING
  TextShadow(SkColor color, SkPoint offset, double blur_sigma);
#else
  TextShadow(uint32_t color, RSPoint offset, double blur_sigma);
#endif

  bool operator==(const TextShadow& other) const;

  bool operator!=(const TextShadow& other) const;

  bool hasShadow() const;
};

}  // namespace txt

#endif  // LIB_TXT_SRC_TEXT_SHADOW_H_
