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

#include <minikin/MinikinFont.h>

#include "flutter/fml/macros.h"
#ifndef USE_ROSEN_DRAWING
#include "third_party/skia/include/core/SkPaint.h"
#include "third_party/skia/include/core/SkTypeface.h"
#else
#include "drawing.h"
#endif

namespace txt {

class FontSkia : public minikin::MinikinFont {
 public:
#ifndef USE_ROSEN_DRAWING
  explicit FontSkia(sk_sp<SkTypeface> typeface);
#else
  explicit FontSkia(std::shared_ptr<RSTypeface> typeface);
#endif

  ~FontSkia();

  float GetHorizontalAdvance(uint32_t glyph_id,
                             const minikin::MinikinPaint& paint) const override;

  void GetBounds(minikin::MinikinRect* bounds,
                 uint32_t glyph_id,
                 const minikin::MinikinPaint& paint) const override;

  hb_face_t* CreateHarfBuzzFace() const override;

  const std::vector<minikin::FontVariation>& GetAxes() const override;

#ifndef USE_ROSEN_DRAWING
  const sk_sp<SkTypeface>& GetSkTypeface() const;
#else
  const std::shared_ptr<RSTypeface>& GetSkTypeface() const;
#endif

 private:
#ifndef USE_ROSEN_DRAWING
  sk_sp<SkTypeface> typeface_;
#else
  std::shared_ptr<RSTypeface> typeface_;
#endif
  std::vector<minikin::FontVariation> variations_;

  FML_DISALLOW_COPY_AND_ASSIGN(FontSkia);
};

}  // namespace txt
