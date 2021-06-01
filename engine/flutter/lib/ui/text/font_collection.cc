// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/text/font_collection.h"

#include <mutex>

#include "flutter/lib/ui/text/asset_manager_font_provider.h"
#include "flutter/lib/ui/ui_dart_state.h"
#include "flutter/lib/ui/window/window.h"
#include "flutter/runtime/test_font_data.h"
#include "third_party/skia/include/core/SkFontMgr.h"
#include "third_party/skia/include/core/SkGraphics.h"
#include "third_party/skia/include/core/SkStream.h"
#include "third_party/skia/include/core/SkTypeface.h"
#include "txt/asset_font_manager.h"
#include "txt/test_font_manager.h"

namespace flutter {

FontCollection::FontCollection()
    : collection_(std::make_shared<txt::FontCollection>()) {
  collection_->SetupDefaultFontManager();

  dynamic_font_manager_ = sk_make_sp<txt::DynamicFontManager>();
  collection_->SetDynamicFontManager(dynamic_font_manager_);
}

FontCollection::~FontCollection() {
  collection_.reset();
  SkGraphics::PurgeFontCache();
}

void FontCollection::RegisterNatives(tonic::DartLibraryNatives* natives) {
}

std::shared_ptr<txt::FontCollection> FontCollection::GetFontCollection() const {
  return collection_;
}

void FontCollection::RegisterFonts(
    std::shared_ptr<AssetManager> asset_manager) {
}

void FontCollection::RegisterTestFonts() {
}

void FontCollection::LoadFontFromList(const uint8_t* font_data,
                                      int length,
                                      std::string family_name) {
  std::unique_ptr<SkStreamAsset> font_stream =
      std::make_unique<SkMemoryStream>(font_data, length, true);
  sk_sp<SkTypeface> typeface =
      SkTypeface::MakeFromStream(std::move(font_stream));
  txt::TypefaceFontAssetProvider& font_provider =
      dynamic_font_manager_->font_provider();
  if (family_name.empty()) {
    font_provider.RegisterTypeface(typeface);
  } else {
    font_provider.RegisterTypeface(typeface, family_name);
  }
  collection_->ClearFontFamilyCache();
}

}  // namespace flutter
