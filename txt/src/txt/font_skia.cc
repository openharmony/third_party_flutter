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

#include "font_skia.h"

#include <minikin/MinikinFont.h>

#include "third_party/skia/include/core/SkFont.h"

namespace txt {
namespace {

hb_blob_t* GetTable(hb_face_t* face, hb_tag_t tag, void* context) {
#ifndef USE_ROSEN_DRAWING
  SkTypeface* typeface = reinterpret_cast<SkTypeface*>(context);

  const size_t table_size = typeface->getTableSize(tag);
#else
  RSTypeface* typeface = reinterpret_cast<RSTypeface*>(context);

  const size_t table_size = typeface->GetTableSize(tag);
#endif
  if (table_size == 0)
    return nullptr;
  void* buffer = malloc(table_size);
  if (buffer == nullptr)
    return nullptr;

#ifndef USE_ROSEN_DRAWING
  size_t actual_size = typeface->getTableData(tag, 0, table_size, buffer);
#else
  size_t actual_size = typeface->GetTableData(tag, 0, table_size, buffer);
#endif
  if (table_size != actual_size) {
    free(buffer);
    return nullptr;
  }
  return hb_blob_create(reinterpret_cast<char*>(buffer), table_size,
                        HB_MEMORY_MODE_WRITABLE, buffer, free);
}

}  // namespace

#ifndef USE_ROSEN_DRAWING
FontSkia::FontSkia(sk_sp<SkTypeface> typeface)
    : MinikinFont(typeface->uniqueID()), typeface_(std::move(typeface)) {}
#else
FontSkia::FontSkia(std::shared_ptr<RSTypeface> typeface)
    : MinikinFont(typeface->UniqueID()), typeface_(std::move(typeface)) {}
#endif

FontSkia::~FontSkia() = default;

#ifndef USE_ROSEN_DRAWING
static void FontSkia_SetSkiaFont(sk_sp<SkTypeface> typeface,
                                 SkFont* skFont,
                                 const minikin::MinikinPaint& paint) {
  skFont->setTypeface(std::move(typeface));
  skFont->setLinearMetrics((paint.paintFlags & minikin::LinearTextFlag) != 0);
  // TODO: set more paint parameters from Minikin
  skFont->setSize(paint.size);
}

float FontSkia::GetHorizontalAdvance(uint32_t glyph_id,
                                     const minikin::MinikinPaint& paint) const {
  SkFont skFont;
  uint16_t glyph16 = glyph_id;
  SkScalar skWidth;
  FontSkia_SetSkiaFont(typeface_, &skFont, paint);
  skFont.getWidths(&glyph16, 1, &skWidth);
  return skWidth;
}

void FontSkia::GetBounds(minikin::MinikinRect* bounds,
                         uint32_t glyph_id,
                         const minikin::MinikinPaint& paint) const {
  SkFont skFont;
  uint16_t glyph16 = glyph_id;
  SkRect skBounds;
  FontSkia_SetSkiaFont(typeface_, &skFont, paint);
  skFont.getWidths(&glyph16, 1, NULL, &skBounds);
  bounds->mLeft = skBounds.fLeft;
  bounds->mTop = skBounds.fTop;
  bounds->mRight = skBounds.fRight;
  bounds->mBottom = skBounds.fBottom;
}
#else
static void FontSkia_SetSkiaFont(std::shared_ptr<RSTypeface> typeface,
                                 RSFont* rsFont,
                                 const minikin::MinikinPaint& paint) {
  rsFont->SetTypeface(std::move(typeface));
  rsFont->SetLinearMetrics((paint.paintFlags & minikin::LinearTextFlag) != 0);
  // TODO: set more paint parameters from Minikin
  rsFont->SetSize(paint.size);
}

float FontSkia::GetHorizontalAdvance(uint32_t glyph_id,
                                     const minikin::MinikinPaint& paint) const {
  RSFont rsFont;
  uint16_t glyph16 = glyph_id;
  RSScalar rsWidth;
  FontSkia_SetSkiaFont(typeface_, &rsFont, paint);
  rsFont.GetWidths(&glyph16, 1, &rsWidth);
  return rsWidth;
}

void FontSkia::GetBounds(minikin::MinikinRect* bounds,
                         uint32_t glyph_id,
                         const minikin::MinikinPaint& paint) const {
  RSFont rsFont;
  uint16_t glyph16 = glyph_id;
  RSRect rsBounds;
  FontSkia_SetSkiaFont(typeface_, &rsFont, paint);
  rsFont.GetWidths(&glyph16, 1, NULL, &rsBounds);
  bounds->mLeft = rsBounds.GetLeft();
  bounds->mTop = rsBounds.GetTop();
  bounds->mRight = rsBounds.GetRight();
  bounds->mBottom = rsBounds.GetBottom();
}
#endif

hb_face_t* FontSkia::CreateHarfBuzzFace() const {
  return hb_face_create_for_tables(GetTable, typeface_.get(), 0);
}

const std::vector<minikin::FontVariation>& FontSkia::GetAxes() const {
  return variations_;
}

#ifndef USE_ROSEN_DRAWING
const sk_sp<SkTypeface>& FontSkia::GetSkTypeface() const {
#else
const std::shared_ptr<RSTypeface>& FontSkia::GetSkTypeface() const {
#endif
  return typeface_;
}

}  // namespace txt
