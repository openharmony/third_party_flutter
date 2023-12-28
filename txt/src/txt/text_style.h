/*
 * Copyright 2017 Google Inc.
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

#ifndef LIB_TXT_SRC_TEXT_STYLE_H_
#define LIB_TXT_SRC_TEXT_STYLE_H_

#include <string>
#include <vector>

#include "font_features.h"
#include "font_style.h"
#include "font_weight.h"
#include "text_baseline.h"
#include "text_decoration.h"
#include "text_shadow.h"
#include "third_party/skia/include/core/SkColor.h"
#include "third_party/skia/include/core/SkPaint.h"

namespace txt {

struct RectStyle {
    uint32_t color = 0;
    double leftTopRadius = 0.0;
    double rightTopRadius = 0.0;
    double rightBottomRadius = 0.0;
    double leftBottomRadius = 0.0;
};

enum class RoundRectType {
  NONE,
  LEFT_ONLY,
  RIGHT_ONLY,
  ALL,
};

class TextStyle {
 public:
#ifndef USE_ROSEN_DRAWING
  SkColor color = SK_ColorWHITE;
#else
  uint32_t color = RSColor::COLOR_WHITE;
#endif
  int decoration = TextDecoration::kNone;
  // Does not make sense to draw a transparent object, so we use it as a default
  // value to indicate no decoration color was set.
#ifndef USE_ROSEN_DRAWING
  SkColor decoration_color = SK_ColorTRANSPARENT;
#else
  uint32_t decoration_color = RSColor::COLOR_TRANSPARENT;
#endif
  TextDecorationStyle decoration_style = TextDecorationStyle::kSolid;
  // Thickness is applied as a multiplier to the default thickness of the font.
  double decoration_thickness_multiplier = 1.0;
  FontWeight font_weight = FontWeight::w400;
  FontStyle font_style = FontStyle::normal;
  TextBaseline text_baseline = TextBaseline::kAlphabetic;
  bool half_leading = false;
  // An ordered list of fonts in order of priority. The first font is more
  // highly preferred than the last font.
  std::vector<std::string> font_families;
  double font_size = 14.0;
  double letter_spacing = 0.0;
  double word_spacing = 0.0;
  double height = 1.0;
  bool has_height_override = false;
  std::string locale;
#ifndef USE_ROSEN_DRAWING
  bool has_background = false;
  SkPaint background;
  bool has_foreground = false;
  SkPaint foreground;
#else
  bool has_background_pen = false;
  RSPen background_pen;
  bool has_background_brush = false;
  RSBrush background_brush;
  bool has_foreground_pen = false;
  RSPen foreground_pen;
  bool has_foreground_brush = false;
  RSBrush foreground_brush;
#endif
  RectStyle backgroundRect = {0, 0.0, 0.0, 0.0, 0.0};
  int styleId = 0;
  // An ordered list of shadows where the first shadow will be drawn first (at
  // the bottom).
  std::vector<TextShadow> text_shadows;
  FontFeatures font_features;

  TextStyle();

  bool equals(const TextStyle& other) const;
};

}  // namespace txt

#endif  // LIB_TXT_SRC_TEXT_STYLE_H_
