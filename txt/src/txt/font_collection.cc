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

#include "font_collection.h"

#include <algorithm>
#include <list>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#if defined(OHOS_PLATFORM) && !defined(OHOS_STANDARD_SYSTEM)
#include "third_party/skia/src/ports/SkFontMgr_ohos.h"
#endif
#include "flutter/fml/logging.h"
#include "flutter/fml/trace_event.h"
#include "font_skia.h"
#include "minikin/Layout.h"
#include "txt/platform.h"
#include "txt/text_style.h"

namespace txt {

namespace {

const std::shared_ptr<minikin::FontFamily> g_null_family;

}  // anonymous namespace

FontCollection::FamilyKey::FamilyKey(const std::vector<std::string>& families,
                                     const std::string& loc) {
  locale = loc;

  std::stringstream stream;
  for_each(families.begin(), families.end(),
           [&stream](const std::string& str) { stream << str << ','; });
  font_families = stream.str();
}

bool FontCollection::FamilyKey::operator==(
    const FontCollection::FamilyKey& other) const {
  return font_families == other.font_families && locale == other.locale;
}

size_t FontCollection::FamilyKey::Hasher::operator()(
    const FontCollection::FamilyKey& key) const {
  return std::hash<std::string>()(key.font_families) ^
         std::hash<std::string>()(key.locale);
}

class TxtFallbackFontProvider
    : public minikin::FontCollection::FallbackFontProvider {
 public:
  TxtFallbackFontProvider(std::shared_ptr<FontCollection> font_collection)
      : font_collection_(font_collection) {}

  virtual const std::shared_ptr<minikin::FontFamily>& matchFallbackFont(
      uint32_t ch,
      std::string locale) {
    std::shared_ptr<FontCollection> fc = font_collection_.lock();
    if (fc) {
      return fc->MatchFallbackFont(ch, locale);
    } else {
      return g_null_family;
    }
  }

  virtual const std::shared_ptr<minikin::FontFamily>& matchFallbackFontFromHwFont(
      uint32_t ch,
      std::string locale) {
#if defined(OHOS_PLATFORM) && !defined(OHOS_STANDARD_SYSTEM)
    std::shared_ptr<FontCollection> fc = font_collection_.lock();
    if (fc) {
      return fc->MatchFallbackFontFromHwFont(ch, locale);
    } else {
      return g_null_family;
    }
#endif
    return g_null_family;
  }

 private:
  std::weak_ptr<FontCollection> font_collection_;
};

FontCollection::FontCollection() : enable_font_fallback_(true) {}

FontCollection::~FontCollection() {
  minikin::Layout::purgeCaches();

#if FLUTTER_ENABLE_SKSHAPER
  if (skt_collection_) {
    skt_collection_->clearCaches();
  }
#endif
}

size_t FontCollection::GetFontManagersCount() const {
  return GetFontManagerOrder().size();
}

void FontCollection::SetupDefaultFontManager() {
  std::lock_guard<std::mutex> lock(fontManagerMutex_);
  default_font_manager_ = GetDefaultFontManager();
}

#ifndef USE_ROSEN_DRAWING
void FontCollection::SetDefaultFontManager(sk_sp<SkFontMgr> font_manager) {
#else
void FontCollection::SetDefaultFontManager(std::shared_ptr<RSFontMgr> font_manager) {
#endif
  std::lock_guard<std::mutex> lock(fontManagerMutex_);
  default_font_manager_ = font_manager;

#if FLUTTER_ENABLE_SKSHAPER
  skt_collection_.reset();
#endif
}

#ifndef USE_ROSEN_DRAWING
void FontCollection::SetAssetFontManager(sk_sp<SkFontMgr> font_manager) {
#else
void FontCollection::SetAssetFontManager(std::shared_ptr<RSFontMgr> font_manager) {
#endif
  asset_font_manager_ = font_manager;

#if FLUTTER_ENABLE_SKSHAPER
  skt_collection_.reset();
#endif
}

#ifndef USE_ROSEN_DRAWING
void FontCollection::SetDynamicFontManager(sk_sp<SkFontMgr> font_manager) {
#else
void FontCollection::SetDynamicFontManager(std::shared_ptr<RSFontMgr> font_manager) {
#endif
  dynamic_font_manager_ = font_manager;

#if FLUTTER_ENABLE_SKSHAPER
  skt_collection_.reset();
#endif
}

#ifndef USE_ROSEN_DRAWING
void FontCollection::SetTestFontManager(sk_sp<SkFontMgr> font_manager) {
#else
void FontCollection::SetTestFontManager(std::shared_ptr<RSFontMgr> font_manager) {
#endif
  test_font_manager_ = font_manager;

#if FLUTTER_ENABLE_SKSHAPER
  skt_collection_.reset();
#endif
}

#ifndef USE_ROSEN_DRAWING
sk_sp<SkFontMgr> FontCollection::GetDefaultFontManagerSafely() const {
#else
std::shared_ptr<RSFontMgr> FontCollection::GetDefaultFontManagerSafely() const {
#endif
  std::lock_guard<std::mutex> lock(fontManagerMutex_);
  return default_font_manager_;
}

// Return the available font managers in the order they should be queried.
#ifndef USE_ROSEN_DRAWING
std::vector<sk_sp<SkFontMgr>> FontCollection::GetFontManagerOrder() const {
  std::vector<sk_sp<SkFontMgr>> order;
#else
std::vector<std::shared_ptr<RSFontMgr>> FontCollection::GetFontManagerOrder() const {
  std::vector<std::shared_ptr<RSFontMgr>> order;
#endif
  if (dynamic_font_manager_)
    order.push_back(dynamic_font_manager_);
  if (asset_font_manager_)
    order.push_back(asset_font_manager_);
  if (test_font_manager_)
    order.push_back(test_font_manager_);
  auto defaultFontManager = GetDefaultFontManagerSafely();
  if (defaultFontManager)
    order.push_back(defaultFontManager);
  return order;
}

void FontCollection::DisableFontFallback() {
  enable_font_fallback_ = false;

#if FLUTTER_ENABLE_SKSHAPER
  if (skt_collection_) {
    skt_collection_->disableFontFallback();
  }
#endif
}

std::shared_ptr<minikin::FontCollection>
FontCollection::GetMinikinFontCollectionForFamilies(
    const std::vector<std::string>& font_families,
    const std::string& locale) {
#if defined(OHOS_PLATFORM) && !defined(OHOS_STANDARD_SYSTEM)
  return GetMinikinFontCollectionForFamiliesWithVariation(font_families, locale);
#endif
  // Look inside the font collections cache first.
  FamilyKey family_key(font_families, locale);
  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto cached = font_collections_cache_.find(family_key);
    if (cached != font_collections_cache_.end()) {
      return cached->second;
    }
  }

  std::vector<std::shared_ptr<minikin::FontFamily>> minikin_families;

  // Search for all user provided font families.
  for (size_t fallback_index = 0; fallback_index < font_families.size();
       fallback_index++) {
    std::shared_ptr<minikin::FontFamily> minikin_family =
        FindFontFamilyInManagers(font_families[fallback_index]);
    if (minikin_family != nullptr) {
      minikin_families.push_back(minikin_family);
    }
  }
  // Search for default font family if no user font families were found.
  if (minikin_families.empty()) {
    const auto default_font_families = GetDefaultFontFamilies();
    for (const auto& family : default_font_families) {
      std::shared_ptr<minikin::FontFamily> minikin_family =
          FindFontFamilyInManagers(family);
      if (minikin_family != nullptr) {
        minikin_families.push_back(minikin_family);
        break;
      }
    }
  }
  // Default font family also not found. We fail to get a FontCollection.
  if (minikin_families.empty()) {
    std::lock_guard<std::mutex> lock(mutex_);
    font_collections_cache_[family_key] = nullptr;
    return nullptr;
  }
  if (enable_font_fallback_) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const std::string& fallback_family :
         fallback_fonts_for_locale_[locale]) {
      auto it = fallback_fonts_.find(fallback_family);
      if (it != fallback_fonts_.end()) {
        minikin_families.push_back(it->second);
      }
    }
  }
  // Create the minikin font collection.
  auto font_collection =
      minikin::FontCollection::Create(std::move(minikin_families));
  if (!font_collection) {
    font_collections_cache_[family_key] = nullptr;
    return nullptr;
  }
  if (enable_font_fallback_) {
    font_collection->set_fallback_font_provider(
        std::make_unique<TxtFallbackFontProvider>(shared_from_this()));
  }

  // Cache the font collection for future queries.
  font_collections_cache_[family_key] = font_collection;

  return font_collection;
}

std::shared_ptr<minikin::FontFamily> FontCollection::FindFontFamilyInManagers(
    const std::string& family_name) {
  TRACE_EVENT0("flutter", "FontCollection::FindFontFamilyInManagers");
  // Search for the font family in each font manager.
#ifndef USE_ROSEN_DRAWING
  for (sk_sp<SkFontMgr>& manager : GetFontManagerOrder()) {
#else
  for (std::shared_ptr<RSFontMgr>& manager : GetFontManagerOrder()) {
#endif
    std::shared_ptr<minikin::FontFamily> minikin_family =
        CreateMinikinFontFamily(manager, family_name);
    if (!minikin_family)
      continue;
    return minikin_family;
  }
  return nullptr;
}

void FontCollection::SortSkTypefaces(
#ifndef USE_ROSEN_DRAWING
    std::vector<sk_sp<SkTypeface>>& sk_typefaces) {
#else
    std::vector<std::shared_ptr<RSTypeface>>& sk_typefaces) {
#endif
  std::sort(
      sk_typefaces.begin(), sk_typefaces.end(),
#ifndef USE_ROSEN_DRAWING
      [](const sk_sp<SkTypeface>& a, const sk_sp<SkTypeface>& b) {
        SkFontStyle a_style = a->fontStyle();
        SkFontStyle b_style = b->fontStyle();

        int a_delta = std::abs(a_style.width() - SkFontStyle::kNormal_Width);
        int b_delta = std::abs(b_style.width() - SkFontStyle::kNormal_Width);
#else
      [](const std::shared_ptr<RSTypeface>& a, const std::shared_ptr<RSTypeface>& b) {
        RSFontStyle a_style = a->GetFontStyle();
        RSFontStyle b_style = b->GetFontStyle();

        int a_delta = std::abs(a_style.GetWidth() - RSFontStyle::NORMAL_WIDTH);
        int b_delta = std::abs(b_style.GetWidth() - RSFontStyle::NORMAL_WIDTH);
#endif

        if (a_delta != b_delta) {
          // If a family name query is so generic it ends up bringing in fonts
          // of multiple widths (e.g. condensed, expanded), opt to be
          // conservative and select the most standard width.
          //
          // If a specific width is desired, it should be be narrowed down via
          // the family name.
          //
          // The font weights are also sorted lightest to heaviest but Flutter
          // APIs have the weight specified to narrow it down later. The width
          // ordering here is more consequential since TextStyle doesn't have
          // letter width APIs.
          return a_delta < b_delta;
#ifndef USE_ROSEN_DRAWING
        } else if (a_style.width() != b_style.width()) {
          // However, if the 2 fonts are equidistant from the "normal" width,
          // just arbitrarily but consistently return the more condensed font.
          return a_style.width() < b_style.width();
        } else if (a_style.weight() != b_style.weight()) {
          return a_style.weight() < b_style.weight();
        } else {
          return a_style.slant() < b_style.slant();
#else
        } else if (a_style.GetWidth() != b_style.GetWidth()) {
          // However, if the 2 fonts are equidistant from the "normal" width,
          // just arbitrarily but consistently return the more condensed font.
          return a_style.GetWidth() < b_style.GetWidth();
        } else if (a_style.GetWeight() != b_style.GetWeight()) {
          return a_style.GetWeight() < b_style.GetWeight();
        } else {
          return a_style.GetSlant() < b_style.GetSlant();
#endif
        }
        // Use a cascade of conditions so results are consistent each time.
      });
}

std::shared_ptr<minikin::FontFamily> FontCollection::CreateMinikinFontFamily(
#ifndef USE_ROSEN_DRAWING
    const sk_sp<SkFontMgr>& manager,
#else
    const std::shared_ptr<RSFontMgr>& manager,
#endif
    const std::string& family_name) {
  TRACE_EVENT1("flutter", "FontCollection::CreateMinikinFontFamily",
               "family_name", family_name.c_str());
#ifndef USE_ROSEN_DRAWING
  sk_sp<SkFontStyleSet> font_style_set(
      manager->matchFamily(family_name.c_str()));
  if (font_style_set == nullptr || font_style_set->count() == 0) {
#else
  std::shared_ptr<RSFontStyleSet> font_style_set(
      manager->MatchFamily(family_name.c_str()));
  if (font_style_set == nullptr || font_style_set->Count() == 0) {
#endif
    return nullptr;
  }

#ifndef USE_ROSEN_DRAWING
  std::vector<sk_sp<SkTypeface>> skia_typefaces;
  for (int i = 0; i < font_style_set->count(); ++i) {
    TRACE_EVENT0("flutter", "CreateSkiaTypeface");
    sk_sp<SkTypeface> skia_typeface(
        sk_sp<SkTypeface>(font_style_set->createTypeface(i)));
#else
  std::vector<std::shared_ptr<RSTypeface>> skia_typefaces;
  for (int i = 0; i < font_style_set->Count(); ++i) {
    TRACE_EVENT0("flutter", "CreateSkiaTypeface");
    std::shared_ptr<RSTypeface> skia_typeface(
        font_style_set->CreateTypeface(i));
#endif
    if (skia_typeface != nullptr) {
      skia_typefaces.emplace_back(std::move(skia_typeface));
    }
  }

  if (skia_typefaces.empty()) {
    return nullptr;
  }

  SortSkTypefaces(skia_typefaces);

  std::vector<minikin::Font> minikin_fonts;
#ifndef USE_ROSEN_DRAWING
  for (const sk_sp<SkTypeface>& skia_typeface : skia_typefaces) {
#else
  for (const std::shared_ptr<RSTypeface>& skia_typeface : skia_typefaces) {
#endif
    // Create the minikin font from the skia typeface.
    // Divide by 100 because the weights are given as "100", "200", etc.
    minikin_fonts.emplace_back(
        std::make_shared<FontSkia>(skia_typeface),
#ifndef USE_ROSEN_DRAWING
        minikin::FontStyle{skia_typeface->fontStyle().weight() / 100,
                           skia_typeface->isItalic()});
#else
        minikin::FontStyle{skia_typeface->GetFontStyle().GetWeight() / 100,
                           skia_typeface->GetItalic()});
#endif
  }

  return std::make_shared<minikin::FontFamily>(std::move(minikin_fonts));
}

const std::shared_ptr<minikin::FontFamily>& FontCollection::MatchFallbackFont(
    uint32_t ch,
    std::string locale) {
#if defined(OHOS_PLATFORM) && !defined(OHOS_STANDARD_SYSTEM)
  return MatchFallbackFontWithVariation(ch, locale);
#endif
  // Check if the ch's matched font has been cached. We cache the results of
  // this method as repeated matchFamilyStyleCharacter calls can become
  // extremely laggy when typing a large number of complex emojis.
  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto lookup = fallback_match_cache_.find(ch);
    if (lookup != fallback_match_cache_.end()) {
      return *lookup->second;
    }
  }
  const std::shared_ptr<minikin::FontFamily>* match =
      &DoMatchFallbackFont(ch, locale);
  {
    std::lock_guard<std::mutex> lock(mutex_);
    fallback_match_cache_.insert(std::make_pair(ch, match));
  }
  return *match;
}

const std::shared_ptr<minikin::FontFamily>& FontCollection::DoMatchFallbackFont(
    uint32_t ch,
    std::string locale) {
#ifndef USE_ROSEN_DRAWING
  for (const sk_sp<SkFontMgr>& manager : GetFontManagerOrder()) {
#else
  for (const std::shared_ptr<RSFontMgr>& manager : GetFontManagerOrder()) {
#endif
    std::vector<const char*> bcp47;
    if (!locale.empty())
      bcp47.push_back(locale.c_str());
#ifndef USE_ROSEN_DRAWING
    sk_sp<SkTypeface> typeface(manager->matchFamilyStyleCharacter(
        0, SkFontStyle(), bcp47.data(), bcp47.size(), ch));
#else
    std::shared_ptr<RSTypeface> typeface(manager->MatchFamilyStyleCharacter(
        0, RSFontStyle(), bcp47.data(), bcp47.size(), ch));
#endif
    if (!typeface)
      continue;

#ifndef USE_ROSEN_DRAWING
    SkString sk_family_name;
    typeface->getFamilyName(&sk_family_name);
    std::string family_name(sk_family_name.c_str());
#else
    std::string family_name = typeface->GetFamilyName();
#endif
    {
	  std::lock_guard<std::mutex> lock(mutex_);
	  if (std::find(fallback_fonts_for_locale_[locale].begin(),
	                fallback_fonts_for_locale_[locale].end(),
	                family_name) == fallback_fonts_for_locale_[locale].end())
	    fallback_fonts_for_locale_[locale].push_back(family_name);
    }
    return GetFallbackFontFamily(manager, family_name);
  }
  return g_null_family;
}

const std::shared_ptr<minikin::FontFamily>&
#ifndef USE_ROSEN_DRAWING
FontCollection::GetFallbackFontFamily(const sk_sp<SkFontMgr>& manager,
#else
FontCollection::GetFallbackFontFamily(const std::shared_ptr<RSFontMgr>& manager,
#endif
                                      const std::string& family_name) {
  TRACE_EVENT0("flutter", "FontCollection::GetFallbackFontFamily");
  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto fallback_it = fallback_fonts_.find(family_name);
    if (fallback_it != fallback_fonts_.end()) {
      return fallback_it->second;
    }
  }

  std::shared_ptr<minikin::FontFamily> minikin_family =
      CreateMinikinFontFamily(manager, family_name);
  if (!minikin_family)
    return g_null_family;

  std::lock_guard<std::mutex> lock(mutex_);
  auto insert_it =
    fallback_fonts_.insert(std::make_pair(family_name, minikin_family));

  // Clear the cache to force creation of new font collections that will
  // include this fallback font.
  font_collections_cache_.clear();
  return insert_it.first->second;
}

void FontCollection::ClearFontFamilyCache() {
  std::lock_guard<std::mutex> lock(mutex_);
  font_collections_cache_.clear();

#if FLUTTER_ENABLE_SKSHAPER
  if (skt_collection_) {
    skt_collection_->clearCaches();
  }
#endif
}

void FontCollection::VaryFontCollectionWithFontWeightScale(float font_weight_scale) {
  if (font_weight_scale > 0.0f && font_weight_scale != font_weight_scale_) {
    decltype(fallback_fonts_) fallback_fonts;
    std::lock_guard<std::mutex> lock(mutex_);
    font_weight_scale_ = font_weight_scale;
    varied_fonts_.clear();
    fallback_fonts = std::move(fallback_fonts_);
    fallback_match_cache_.clear();
    fallback_fonts_for_locale_.clear();
  }
}

void FontCollection::LoadSystemFont() {
#if defined(OHOS_PLATFORM) && !defined(OHOS_STANDARD_SYSTEM)
  {
    decltype(fallback_fonts_) fallback_fonts;
    std::lock_guard<std::mutex> lock(mutex_);
    varied_fonts_.clear();
    fallback_fonts = std::move(fallback_fonts_);
    fallback_match_cache_.clear();
    fallback_fonts_for_locale_.clear();
  }

  SetupDefaultFontManager();

#endif
}

void FontCollection::SetIsZawgyiMyanmar(bool is_zawgyi_myanmar) {
  if (is_zawgyi_myanmar_ == is_zawgyi_myanmar) {
    return;
  }
  is_zawgyi_myanmar_ = is_zawgyi_myanmar;
  LoadSystemFont();
}

#if defined(OHOS_PLATFORM) && !defined(OHOS_STANDARD_SYSTEM)
// Return the available font managers in the order they should be queried with
// type.
std::vector<std::pair<sk_sp<SkFontMgr>, txt::FontManagerType>>
FontCollection::GetFontManagerOrderWithType() const {
  std::vector<std::pair<sk_sp<SkFontMgr>, txt::FontManagerType>> order;
  if (dynamic_font_manager_)
    order.push_back(
        std::make_pair(dynamic_font_manager_, txt::FontManagerType::DYNAMIC));
  if (asset_font_manager_)
    order.push_back(
        std::make_pair(asset_font_manager_, txt::FontManagerType::ASSET));
  if (test_font_manager_)
    order.push_back(
        std::make_pair(test_font_manager_, txt::FontManagerType::TEST));
  auto defaultFontManager = GetDefaultFontManagerSafely();
  if (defaultFontManager)
    order.push_back(
        std::make_pair(defaultFontManager, GetDefaultFontManagerType()));
  return order;
}

std::shared_ptr<minikin::FontCollection>
FontCollection::GetMinikinFontCollectionForFamiliesWithVariation(
    const std::vector<std::string>& font_families,
    const std::string& locale) {
  // Look inside the font collections cache first.
  FamilyKey family_key(font_families, locale);
  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = std::find(varied_fonts_.begin(), varied_fonts_.end(), family_key);
    if (iter != varied_fonts_.end()) {
      auto cached = font_collections_cache_.find(family_key);
      if (cached != font_collections_cache_.end()) {
        return cached->second;
      }
      varied_fonts_.erase(iter);
    }
  }

  std::vector<std::shared_ptr<minikin::FontFamily>> minikin_families;
  // Search for all user provided font families.
  for (size_t fallback_index = 0; fallback_index < font_families.size();
       fallback_index++) {
    std::shared_ptr<minikin::FontFamily> minikin_family =
        FindFontFamilyInManagersWithType(font_families[fallback_index]);
    if (minikin_family != nullptr) {
      minikin_families.push_back(minikin_family);
    }
  }
  // Search for default font family if no user font families were found.
  if (minikin_families.empty()) {
    const auto default_font_family = GetDefaultFontFamily();
    std::shared_ptr<minikin::FontFamily> minikin_family =
        FindFontFamilyInManagersWithType(default_font_family);
    if (minikin_family != nullptr) {
      minikin_families.push_back(minikin_family);
    }
  }
  // Default font family also not found. We fail to get a FontCollection.
  if (minikin_families.empty()) {
    std::shared_ptr<minikin::FontCollection> tmp_font_collection;
    std::lock_guard<std::mutex> lock(mutex_);
    std::swap(font_collections_cache_[family_key], tmp_font_collection);
    return nullptr;
  }
  if (enable_font_fallback_) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (std::string fallback_family : fallback_fonts_for_locale_[locale]) {
      auto it = fallback_fonts_.find(fallback_family);
      if (it != fallback_fonts_.end()) {
        minikin_families.push_back(it->second);
      }
    }
  }
  // Create the minikin font collection.
  auto font_collection =
      std::make_shared<minikin::FontCollection>(std::move(minikin_families));
  font_collection->SetIsZawgyiMyanmar(is_zawgyi_myanmar_);

  if (enable_font_fallback_) {
    font_collection->set_fallback_font_provider(
        std::make_unique<TxtFallbackFontProvider>(shared_from_this()));
  }

  {
    std::shared_ptr<minikin::FontCollection> tmp_font_collection =
        font_collection;
    std::lock_guard<std::mutex> lock(mutex_);
    // Cache the font collection for future queries.
    std::swap(font_collections_cache_[family_key], tmp_font_collection);
    varied_fonts_.emplace_back(family_key);
  }

  return font_collection;
}

std::shared_ptr<minikin::FontFamily>
FontCollection::FindFontFamilyInManagersWithType(
    const std::string& family_name) {
  TRACE_EVENT0("flutter", "FontCollection::FindFontFamilyInManagersWithType");
  // Search for the font family in each font manager.
  for (std::pair<sk_sp<SkFontMgr>, FontManagerType>& managerPair :
       GetFontManagerOrderWithType()) {
    std::shared_ptr<minikin::FontFamily> minikin_family;
    if (managerPair.second == FontManagerType::DEFAULT_OHOS) {
      minikin_family =
          CreateMinikinFontFamilyForOHOS(managerPair.first, family_name);
    } else {
      minikin_family =
          CreateMinikinFontFamilyExceptOHOS(managerPair.first, family_name);
    }
    if (!minikin_family)
      continue;
    return minikin_family;
  }
  return nullptr;
}

std::shared_ptr<minikin::FontFamily>
FontCollection::CreateMinikinFontFamilyForOHOS(
    const sk_sp<SkFontMgr>& manager,
    const std::string& family_name) {
  TRACE_EVENT1("flutter", "FontCollection::CreateMinikinFontFamilyForOHOS",
               "family_name", family_name.c_str());
  sk_sp<SkFontStyleSet> font_style_set(
      manager->matchFamily(family_name.c_str()));
  if (font_style_set == nullptr || font_style_set->count() == 0) {
    return nullptr;
  }

  auto font_style_set_ohos =
      reinterpret_cast<SkFontStyleSet_OHOS*>(font_style_set.get());
  if (font_style_set_ohos == nullptr) {
    return CreateMinikinFontFamilyExceptOHOS(manager, family_name);
  }

  std::vector<sk_sp<SkTypeface>> skia_typefaces;
  for (int i = 0; i < font_style_set_ohos->count(); ++i) {
    TRACE_EVENT0("flutter", "CreateSkiaTypeface");
    sk_sp<SkTypeface> skia_typeface(
        sk_sp<SkTypeface>(font_style_set_ohos->createTypeface(i)));
    float wghtValue = font_style_set_ohos->getWghtValue(i);
    if (wghtValue <= 0.0f) {
      wghtValue = font_style_set_ohos->getFontWeight(i);
    }
    VaryTypeface(skia_typeface, wghtValue);
    if (skia_typeface != nullptr) {
      skia_typefaces.emplace_back(std::move(skia_typeface));
    }
  }

  int index = 0;
  std::vector<minikin::Font> minikin_fonts;
  for (const sk_sp<SkTypeface>& skia_typeface : skia_typefaces) {
    // Create the minikin font from the skia typeface.
    // Divide by 100 because the weights are given as "100", "200", etc.
    minikin_fonts.emplace_back(
        std::make_shared<FontSkia>(skia_typeface),
        minikin::FontStyle{font_style_set_ohos->getFontWeight(index++) / 100,
                           skia_typeface->isItalic()});
  }

  return std::make_shared<minikin::FontFamily>(std::move(minikin_fonts));
}

std::shared_ptr<minikin::FontFamily>
FontCollection::CreateMinikinFontFamilyExceptOHOS(
    const sk_sp<SkFontMgr>& manager,
    const std::string& family_name) {
  TRACE_EVENT1("flutter",
               "FontCollection::CreateMinikinFontFamilyExceptOHOS",
               "family_name", family_name.c_str());
  sk_sp<SkFontStyleSet> font_style_set(
      manager->matchFamily(family_name.c_str()));
  if (font_style_set == nullptr || font_style_set->count() == 0) {
    return nullptr;
  }

  std::vector<sk_sp<SkTypeface>> skia_typefaces;
  for (int i = 0; i < font_style_set->count(); ++i) {
    TRACE_EVENT0("flutter", "CreateSkiaTypeface");
    sk_sp<SkTypeface> skia_typeface(
        sk_sp<SkTypeface>(font_style_set->createTypeface(i)));
    VaryTypeface(skia_typeface, skia_typeface->fontStyle().weight());
    if (skia_typeface != nullptr) {
      skia_typefaces.emplace_back(std::move(skia_typeface));
    }
  }

  std::sort(skia_typefaces.begin(), skia_typefaces.end(),
            [](const sk_sp<SkTypeface>& a, const sk_sp<SkTypeface>& b) {
              SkFontStyle a_style = a->fontStyle();
              SkFontStyle b_style = b->fontStyle();
              return (a_style.weight() != b_style.weight())
                         ? a_style.weight() < b_style.weight()
                         : a_style.slant() < b_style.slant();
            });

  std::vector<minikin::Font> minikin_fonts;
  for (const sk_sp<SkTypeface>& skia_typeface : skia_typefaces) {
    // Create the minikin font from the skia typeface.
    // Divide by 100 because the weights are given as "100", "200", etc.
    minikin_fonts.emplace_back(
        std::make_shared<FontSkia>(skia_typeface),
        minikin::FontStyle{skia_typeface->fontStyle().weight() / 100,
                           skia_typeface->GetItalic()});
  }

  return std::make_shared<minikin::FontFamily>(std::move(minikin_fonts));
}

void FontCollection::VaryTypeface(sk_sp<SkTypeface>& typeface, float wght) {
  if (font_weight_scale_ <= 0.0f) {
    return;
  }
  // Value of wght is between 0.0f and 1000.0f, and must be greater than 0.0f,
  // default value is 400.0f.
  if (wght <= 0.0) {
    wght = 400.0f;
  }
  float wghtValue = std::min(wght * font_weight_scale_, 1000.0f);
  SkFontArguments params;
  int ttcIndex;
  std::unique_ptr<SkStreamAsset> stream(typeface->openStream(&ttcIndex));
  params.setCollectionIndex(ttcIndex);
  std::vector<minikin::FontVariation> variations = {
      {minikin::MinikinFont::MakeTag('w', 'g', 'h', 't'), wghtValue}};
  std::vector<SkFontArguments::Axis> skAxes;
  skAxes.resize(variations.size());
  for (size_t i = 0; i < variations.size(); i++) {
    skAxes[i].fTag = variations[i].axisTag;
    skAxes[i].fStyleValue = variations[i].value;
  }
  params.setAxes(skAxes.data(), skAxes.size());
  sk_sp<SkFontMgr> fm(SkFontMgr::RefDefault());
  typeface = fm->makeFromStream(std::move(stream), params);
}

const std::shared_ptr<minikin::FontFamily>&
FontCollection::MatchFallbackFontWithVariation(uint32_t ch,
                                               std::string locale) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto lookup = fallback_match_cache_.find(ch);
    if (lookup != fallback_match_cache_.end()) {
      return *lookup->second;
    }
  }
  const std::shared_ptr<minikin::FontFamily>* match =
      &DoMatchFallbackFontWithVariation(ch, locale);
  {
    std::lock_guard<std::mutex> lock(mutex_);
    fallback_match_cache_.insert(std::make_pair(ch, match));
  }
  return *match;
}

const std::shared_ptr<minikin::FontFamily>&
FontCollection::DoMatchFallbackFontWithVariation(uint32_t ch,
                                                 std::string locale) {
  for (const std::pair<sk_sp<SkFontMgr>, FontManagerType>& managerPair :
       GetFontManagerOrderWithType()) {
    std::vector<const char*> bcp47;
    if (!locale.empty())
      bcp47.push_back(locale.c_str());

    if (managerPair.second == FontManagerType::DEFAULT_OHOS) {
      auto ohosManager = reinterpret_cast<SkFontMgr_OHOS*>(managerPair.first.get());
      if (ohosManager == nullptr)
        continue;

      SkString sk_family_name = ohosManager->onMatchFamilyStyleCharacterOHOS(
        0, SkFontStyle(), bcp47.data(), bcp47.size(), ch);
      if(sk_family_name.isEmpty())
        continue;

      return GetFallbackFontFamilyForOHOS(managerPair.first, std::string(sk_family_name.c_str()));
    } else {
      sk_sp<SkTypeface> typeface(managerPair.first->matchFamilyStyleCharacter(
        0, SkFontStyle(), bcp47.data(), bcp47.size(), ch));
      if (!typeface)
        continue;

      SkString sk_family_name;
      typeface->getFamilyName(&sk_family_name);
      std::string family_name(sk_family_name.c_str());

      {
        std::lock_guard<std::mutex> lock(mutex_);
        fallback_fonts_for_locale_[locale].insert(family_name);
      }

      return GetFallbackFontFamily(managerPair.first, family_name);
    }
  }
  return g_null_family;
}

const std::shared_ptr<minikin::FontFamily>&
FontCollection::GetFallbackFontFamilyForOHOS(
    const sk_sp<SkFontMgr>& manager,
    const std::string& family_name) {
  TRACE_EVENT0("flutter", "FontCollection::GetFallbackFontFamily");
  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto fallback_it = fallback_fonts_.find(family_name);
    if (fallback_it != fallback_fonts_.end()) {
      return fallback_it->second;
    }
  }

  std::shared_ptr<minikin::FontFamily> minikin_family =
      CreateMinikinFontFamilyForOHOS(manager, family_name);
  if (!minikin_family) {
    minikin_family = CreateMinikinFontFamilyExceptOHOS(manager, family_name);
  }
  if (!minikin_family)
    return g_null_family;

  {
    decltype(font_collections_cache_) font_collections_cache;
    std::lock_guard<std::mutex> lock(mutex_);
    auto insert_it =
        fallback_fonts_.insert(std::make_pair(family_name, minikin_family));

    // Clear the cache to force creation of new font collections that will
    // include this fallback font.
    std::swap(font_collections_cache_, font_collections_cache);

    return insert_it.first->second;
  }
}

const std::shared_ptr<minikin::FontFamily>& FontCollection::MatchFallbackFontFromHwFont(
    uint32_t ch,
    std::string locale) {
  const std::shared_ptr<minikin::FontFamily>* match =
      &DoMatchFallbackFontFromHwFont(ch, locale);
  return *match;
}

const std::shared_ptr<minikin::FontFamily>&
FontCollection::DoMatchFallbackFontFromHwFont(uint32_t ch,
                                              std::string locale) {
  for (const std::pair<sk_sp<SkFontMgr>, FontManagerType>& managerPair :
  GetFontManagerOrderWithType()) {
    std::vector<const char*> bcp47;
    if (!locale.empty())
      bcp47.push_back(locale.c_str());

    if (managerPair.second == FontManagerType::DEFAULT_OHOS) {
      auto ohosManager = reinterpret_cast<SkFontMgr_OHOS*>(managerPair.first.get());
      if (ohosManager == nullptr)
        continue;

      SkString sk_family_name = ohosManager->onMatchFamilyStyleCharacterHwFont(
          0, SkFontStyle(), bcp47.data(), bcp47.size(), ch);
      if(sk_family_name.isEmpty())
        continue;

      return GetFallbackFontFamilyForOHOS(managerPair.first, std::string(sk_family_name.c_str()));
    }
  }
  return g_null_family;
}
#endif
#if FLUTTER_ENABLE_SKSHAPER

sk_sp<skia::textlayout::FontCollection>
FontCollection::CreateSktFontCollection() {
  if (!skt_collection_) {
    skt_collection_ = sk_make_sp<skia::textlayout::FontCollection>();

    std::vector<SkString> default_font_families;
    for (const std::string& family : GetDefaultFontFamilies()) {
      default_font_families.emplace_back(family);
    }
    skt_collection_->setDefaultFontManager(GetDefaultFontManagerSafely(),
                                           default_font_families);
    skt_collection_->setAssetFontManager(asset_font_manager_);
    skt_collection_->setDynamicFontManager(dynamic_font_manager_);
    skt_collection_->setTestFontManager(test_font_manager_);
    if (!enable_font_fallback_) {
      skt_collection_->disableFontFallback();
    }
  }

  return skt_collection_;
}

#endif  // FLUTTER_ENABLE_SKSHAPER

}  // namespace txt
