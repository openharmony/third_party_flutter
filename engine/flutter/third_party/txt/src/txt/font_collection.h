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

#ifndef LIB_TXT_SRC_FONT_COLLECTION_H_
#define LIB_TXT_SRC_FONT_COLLECTION_H_

#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include "flutter/fml/macros.h"
#include "minikin/FontCollection.h"
#include "minikin/FontFamily.h"
#include "third_party/googletest/googletest/include/gtest/gtest_prod.h"  // nogncheck
#include "third_party/skia/include/core/SkFontMgr.h"
#include "third_party/skia/include/core/SkRefCnt.h"
#include "txt/asset_font_manager.h"
#include "txt/platform.h"
#include "txt/text_style.h"

#if FLUTTER_ENABLE_SKSHAPER
#include "third_party/skia/modules/skparagraph/include/FontCollection.h"
#endif

namespace txt {

class FontCollection : public std::enable_shared_from_this<FontCollection> {
 public:
  FontCollection();

  ~FontCollection();

  size_t GetFontManagersCount() const;

  void SetupDefaultFontManager();
  void SetDefaultFontManager(sk_sp<SkFontMgr> font_manager);
  void SetAssetFontManager(sk_sp<SkFontMgr> font_manager);
  void SetDynamicFontManager(sk_sp<SkFontMgr> font_manager);
  void SetTestFontManager(sk_sp<SkFontMgr> font_manager);

  std::shared_ptr<minikin::FontCollection> GetMinikinFontCollectionForFamilies(
      const std::vector<std::string>& font_families,
      const std::string& locale);

  // Provides a FontFamily that contains glyphs for ch. This caches previously
  // matched fonts. Also see FontCollection::DoMatchFallbackFont.
  const std::shared_ptr<minikin::FontFamily>& MatchFallbackFont(
      uint32_t ch,
      std::string locale);

  // Do not provide alternative fonts that can match characters which are
  // missing from the requested font family.
  void DisableFontFallback();

  // Remove all entries in the font family cache.
  void ClearFontFamilyCache();

  // Vary font collection with font weight scale.
  void VaryFontCollectionWithFontWeightScale(float font_weight_scale);

  void LoadSystemFont();

  void SetIsZawgyiMyanmar(bool is_zawgyi_myanmar);

#if FLUTTER_ENABLE_SKSHAPER

  // Construct a Skia text layout FontCollection based on this collection.
  sk_sp<skia::textlayout::FontCollection> CreateSktFontCollection();

#endif  // FLUTTER_ENABLE_SKSHAPER

#if defined(OHOS_PLATFORM) && !defined(OHOS_STANDARD_SYSTEM)
    const std::shared_ptr<minikin::FontFamily>& MatchFallbackFontFromHwFont(
            uint32_t ch,
            std::string locale);
#endif

 private:
  struct FamilyKey {
    FamilyKey(const std::vector<std::string>& families, const std::string& loc);

    // Concatenated string with all font families.
    std::string font_families;
    std::string locale;

    bool operator==(const FamilyKey& other) const;

    struct Hasher {
      size_t operator()(const FamilyKey& key) const;
    };
  };

  sk_sp<SkFontMgr> default_font_manager_;
  sk_sp<SkFontMgr> asset_font_manager_;
  sk_sp<SkFontMgr> dynamic_font_manager_;
  sk_sp<SkFontMgr> test_font_manager_;
  std::unordered_map<FamilyKey,
                     std::shared_ptr<minikin::FontCollection>,
                     FamilyKey::Hasher>
      font_collections_cache_;
  // Cache that stores the results of MatchFallbackFont to ensure lag-free emoji
  // font fallback matching.
  std::unordered_map<uint32_t, const std::shared_ptr<minikin::FontFamily>*>
      fallback_match_cache_;
  std::unordered_map<std::string, std::shared_ptr<minikin::FontFamily>>
      fallback_fonts_;
  std::unordered_map<std::string, std::set<std::string>>
      fallback_fonts_for_locale_;
  bool enable_font_fallback_;
  bool is_zawgyi_myanmar_ = false; // whether encoding of Burmese is zawgyi, not unicode.
  float font_weight_scale_ = 1.0f;
  std::vector<FamilyKey> varied_fonts_;

  std::mutex mutex_;
  mutable std::mutex fontManagerMutex_;

  // Performs the actual work of MatchFallbackFont. The result is cached in
  // fallback_match_cache_.
  const std::shared_ptr<minikin::FontFamily>& DoMatchFallbackFont(
      uint32_t ch,
      std::string locale);

  std::vector<sk_sp<SkFontMgr>> GetFontManagerOrder() const;

  std::shared_ptr<minikin::FontFamily> FindFontFamilyInManagers(
      const std::string& family_name);

  std::shared_ptr<minikin::FontFamily> CreateMinikinFontFamily(
      const sk_sp<SkFontMgr>& manager,
      const std::string& family_name);

  const std::shared_ptr<minikin::FontFamily>& GetFallbackFontFamily(
      const sk_sp<SkFontMgr>& manager,
      const std::string& family_name);

  sk_sp<SkFontMgr> GetDefaultFontManagerSafely() const;

#if defined(OHOS_PLATFORM) && !defined(OHOS_STANDARD_SYSTEM)
  std::shared_ptr<minikin::FontCollection>
  GetMinikinFontCollectionForFamiliesWithVariation(
      const std::vector<std::string>& font_families,
      const std::string& locale);

  std::vector<std::pair<sk_sp<SkFontMgr>, txt::FontManagerType>>
  GetFontManagerOrderWithType() const;

  std::shared_ptr<minikin::FontFamily> FindFontFamilyInManagersWithType(
      const std::string& family_name);

  std::shared_ptr<minikin::FontFamily> CreateMinikinFontFamilyForOHOS(
      const sk_sp<SkFontMgr>& manager,
      const std::string& family_name);

  std::shared_ptr<minikin::FontFamily> CreateMinikinFontFamilyExceptOHOS(
      const sk_sp<SkFontMgr>& manager,
      const std::string& family_name);

  void VaryTypeface(sk_sp<SkTypeface>& typeface, float wght);

  // Provides a FontFamily that contains glyphs for ch. This caches previously
  // matched fonts. Also see FontCollection::DoMatchFallbackFontWithVariation.
  const std::shared_ptr<minikin::FontFamily>& MatchFallbackFontWithVariation(
      uint32_t ch,
      std::string locale);

  // Performs the actual work of MatchFallbackFont. The result is cached in
  // fallback_match_cache_.
  const std::shared_ptr<minikin::FontFamily>& DoMatchFallbackFontWithVariation(
      uint32_t ch,
      std::string locale);

  const std::shared_ptr<minikin::FontFamily>& GetFallbackFontFamilyForOHOS(
      const sk_sp<SkFontMgr>& manager,
      const std::string& family_name);

  const std::shared_ptr<minikin::FontFamily>& DoMatchFallbackFontFromHwFont(
      uint32_t ch,
      std::string locale);
#endif

  FML_DISALLOW_COPY_AND_ASSIGN(FontCollection);
};

}  // namespace txt

#endif  // LIB_TXT_SRC_FONT_COLLECTION_H_
