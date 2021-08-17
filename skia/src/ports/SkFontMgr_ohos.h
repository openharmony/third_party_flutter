/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontMgr_ohos_DEFINED
#define SkFontMgr_ohos_DEFINED

#include <algorithm>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkTArray.h"

class SkFontMgr;

enum FontVariants {
    kNone_FontVariant = 0,
    kCompact_FontVariant = 1,
    kElegant_FontVariant = 2,
    kLast_FontVariant = kElegant_FontVariant,
};
typedef uint32_t FontVariant;

struct FontVariationAxis {
    std::string tag;
    float value;
};

struct FontFamilyItem {
    SkTypeface* typeface;
    std::string lang;
    int weight;
    int index;
    int variant;
    std::vector<FontVariationAxis> variationAxes;
    int hwFontFamilyType;
};

struct FamilyAliasObj {
    std::string toName;
    int weight;
};

class SkTypeface_OHOS : public SkWeakRefCnt {
public:
    SkTypeface_OHOS(SkTypeface* typeface,
                       const std::string& familyName,
                       const std::string& lang,
                       FontVariant variantStyle,
                       const std::vector<FontVariationAxis>& variationAxes,
                       int fontWeight,
                       int hwFontFamilyType)
        : fTypeface(typeface)
        , fLang(lang)
        , fVariantStyle(variantStyle)
        , fFamilyName(familyName.c_str())
        , variationAxes(variationAxes)
        , fontWeight(fontWeight)
        , hwFontFamilyType(hwFontFamilyType)
    { }

    ~SkTypeface_OHOS() override = default;

    const SkString& getFamilyName() const {
        return fFamilyName;
    }

    FontVariant getFontVariant() const {
        return fVariantStyle;
    }

    const std::vector<FontVariationAxis>& getFontVariationAxes() const {
        return variationAxes;
    }

    float getWghtValue() const {
        for (auto axis : variationAxes) {
            if (axis.tag == "wght") {
                return axis.value;
            }
        }
        return 0.0f;
    }

    int getFontWeight() const {
        return fontWeight;
    }

    const std::string getLangTag() const {
        return fLang;
    }

    int getHwFontFamilyType() const {
        return hwFontFamilyType;
    }

protected:
    SkTypeface* fTypeface = nullptr;
    std::string fLang;
    const FontVariant fVariantStyle = kNone_FontVariant;
    SkString fFamilyName;
    std::vector<FontVariationAxis> variationAxes;
    int fontWeight = 400; // default font weight is 400.
    int hwFontFamilyType = 0;
};

class SkLanguage {
public:
    SkLanguage() { }
    SkLanguage(const SkString& tag) : fTag(tag) { }
    SkLanguage(const char* tag) : fTag(tag) { }
    SkLanguage(const char* tag, size_t len) : fTag(tag, len) { }
    SkLanguage(const SkLanguage& b) : fTag(b.fTag) { }
    ~SkLanguage() = default;

    /** Gets a BCP 47 language identifier for this SkLanguage.
        @return a BCP 47 language identifier representing this language
    */
    const SkString& getTag() const { return fTag; }

    /** Performs BCP 47 fallback to return an SkLanguage one step more general.
        @return an SkLanguage one step more general
    */
    SkLanguage getParent() const {
        SkASSERT(!fTag.isEmpty());
        const char* tag = fTag.c_str();

        // strip off the rightmost "-.*"
        const char* parentTagEnd = strrchr(tag, '-');
        if (parentTagEnd == nullptr) {
            return SkLanguage();
        }
        size_t parentTagLen = parentTagEnd - tag;
        return SkLanguage(tag, parentTagLen);
    }

    bool operator==(const SkLanguage& b) const {
        return fTag == b.fTag;
    }
    bool operator!=(const SkLanguage& b) const {
        return fTag != b.fTag;
    }
    SkLanguage& operator=(const SkLanguage& b) {
        fTag = b.fTag;
        return *this;
    }

private:
    //! BCP 47 language identifier
    SkString fTag;
};

class SkFontStyleSet_OHOS : public SkFontStyleSet {
public:
    SkFontStyleSet_OHOS(const std::vector<FontFamilyItem>& items, bool fallback = false) {
        for (size_t i = 0; i < items.size(); ++i) {
            const auto& item = items[i];
            auto typeface = item.typeface;
            int fontWeight = item.weight;
            fStyles.push_back().reset(SkRef(typeface));
            SkString skFamilyName;
            typeface->getFamilyName(&skFamilyName);
            std::string familyName(skFamilyName.c_str());
            std::transform(familyName.begin(), familyName.end(), familyName.begin(),
                [](unsigned char c) -> unsigned char { return std::tolower(c); });
            std::vector<FontVariationAxis> variationAxis;
            if (item.variationAxes.size() > 0) {
                for (size_t index = 0; index < item.variationAxes.size(); ++index) {
                    auto variationAxes = item.variationAxes[index];
                    variationAxis.push_back({ .tag = variationAxes.tag, .value = variationAxes.value });
                }
            }
            fStylesHost.push_back().reset(new SkTypeface_OHOS(
                item.typeface, familyName, item.lang, item.variant,
                variationAxis, fontWeight, item.hwFontFamilyType));
        }
    }

    ~SkFontStyleSet_OHOS() override = default;

    int count() override {
        return fStyles.count();
    }

    void getStyle(int index, SkFontStyle* style, SkString* name) override {
        if (index < 0 || fStyles.count() <= index) {
            return;
        }
        if (style) {
            *style = fStyles[index]->fontStyle();
        }
        if (name) {
            name->reset();
        }
    }

    SkTypeface* createTypeface(int index) override {
        if (index < 0 || fStyles.count() <= index) {
            return nullptr;
        }
        return SkRef(fStyles[index].get());
    }

    SkTypeface* matchStyle(const SkFontStyle& pattern) override {
        return static_cast<SkTypeface*>(matchStyleCSS3(pattern));
    }

    bool haveVariant(FontVariant variant) {
        for (int i = 0; i < fStylesHost.count(); ++i) {
            if (fStylesHost[i]->getFontVariant() == variant) {
                return true;
            }
        }
        return false;
    }

    float getWghtValue(int index) {
        if (index < 0 || fStylesHost.count() <= index) {
            return 0.0f;
        }
        return fStylesHost[index]->getWghtValue();
    }

    int getFontWeight(int index) {
        if (index < 0 || fStylesHost.count() <= index) {
            return 400;
        }
        return fStylesHost[index]->getFontWeight();
    }

    bool matchLanguage(const SkString& lang) {
        for (int i = 0; i < fStylesHost.count(); ++i) {
            SkString langTag(fStylesHost[i]->getLangTag().c_str());
            if (langTag.startsWith(lang.c_str())) {
                return true;
            }
        }
        return false;
    }

    int getHwFontFamilyType() {
        if (fStylesHost.count() <= 0) {
            return 0;
        }
        return fStylesHost[0]->getHwFontFamilyType();
    }

private:
    SkTArray<sk_sp<SkTypeface>> fStyles;
    SkTArray<sk_sp<SkTypeface_OHOS>> fStylesHost;
    SkString fFallbackFor;

    friend struct NameToFamily;
    friend class SkFontMgr_OHOS;

    typedef SkFontStyleSet INHERITED;
};

struct NameToFamily {
    SkString name;
    SkFontStyleSet_OHOS* styleSet;
};

SK_API sk_sp<SkFontMgr> SkFontMgr_New_OHOS();

class SkFontMgr_OHOS : public SkFontMgr {
public:
    using BuildFamilyMapCallback = std::function<void(SkTArray<NameToFamily, true>&,
                                                        SkTArray<NameToFamily, true>&,
                                                        SkTArray<sk_sp<SkFontStyleSet_OHOS>>&,
                                                        std::map<std::string, FamilyAliasObj>&)>;

    SkFontMgr_OHOS();
    ~SkFontMgr_OHOS() override = default;

    SkString onMatchFamilyStyleCharacterOHOS(const char familyName[],
                                                    const SkFontStyle& style,
                                                    const char* bcp47[],
                                                    int bcp47Count,
                                                    SkUnichar character);

  SkString onMatchFamilyStyleCharacterHwFont(const char familyName[],
                                           const SkFontStyle& style,
                                           const char* bcp47[],
                                           int bcp47Count,
                                           SkUnichar character);

    static void setBuildFamilyMapCallback(BuildFamilyMapCallback&& callback) {
        buildFamilyMapCallback = std::move(callback);
    }

protected:
    int onCountFamilies() const override;
    void onGetFamilyName(int index, SkString* familyName) const override;
    SkFontStyleSet* onCreateStyleSet(int index) const override;
    SkFontStyleSet* onMatchFamily(const char familyName[]) const override;
    virtual SkTypeface* onMatchFamilyStyle(const char familyName[],
                                           const SkFontStyle& style) const override;
    virtual SkTypeface* onMatchFaceStyle(const SkTypeface* typeface,
                                         const SkFontStyle& style) const override;

    static sk_sp<SkTypeface> find_family_style_character(
            const SkString& familyName,
            const SkTArray<NameToFamily, true>& fallbackNameToFamilyMap,
            const SkFontStyle& style, bool elegant,
            const SkString& langTag, SkUnichar character);

    virtual SkTypeface* onMatchFamilyStyleCharacter(const char familyName[],
                                                    const SkFontStyle& style,
                                                    const char* bcp47[],
                                                    int bcp47Count,
                                                    SkUnichar character) const override;
    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData> data, int ttcIndex) const override;
    sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override;
    sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset> stream,
                                            int ttcIndex) const override;
    sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset> stream,
                                           const SkFontArguments& args) const override;
    sk_sp<SkTypeface> onMakeFromFontData(std::unique_ptr<SkFontData> data) const override;
    sk_sp<SkTypeface> onLegacyMakeTypeface(const char familyName[], SkFontStyle style) const override;

private:
    void buildNameToFamilyMap();
    void findDefaultStyleSet();
    static BuildFamilyMapCallback buildFamilyMapCallback;
    SkString find_family_style_character_ohos(
        const SkString& familyName,
        const SkTArray<NameToFamily, true>& fallbackNameToFamilyMap,
        const SkFontStyle& style, bool elegant,
        const SkString& langTag, SkUnichar character);
    SkString find_family_style_character_hwfont(
        const SkString& familyName,
        const SkTArray<NameToFamily, true>& fallbackNameToFamilyMap,
        const SkFontStyle& style, bool elegant,
        const SkString& langTag, SkUnichar character);
    void addToCache(const NameToFamily* item);
    SkString findFromCache(const SkString& familyName, const SkFontStyle& style, bool elegant,
                           const SkString& langTag, SkUnichar character);

    void addToHwFontCache(const NameToFamily* item);
    SkString findFromHwFontCache(const SkString& familyName, const SkFontStyle& style, bool elegant,
                           const SkString& langTag, SkUnichar character);

    SkTArray<sk_sp<SkFontStyleSet_OHOS>> fStyleSets;
    sk_sp<SkFontStyleSet> fDefaultStyleSet;

    std::mutex mutexCache;
    SkTArray<const NameToFamily*, true> familyMapCache;
    SkTArray<const NameToFamily*, true> hwFontFamilyMapCache;
    SkTArray<NameToFamily, true> fNameToFamilyMap;
    SkTArray<NameToFamily, true> fFallbackNameToFamilyMap;
    std::map<std::string, FamilyAliasObj> fAliasMap;

    typedef SkFontMgr INHERITED;
};

#endif // SkFontMgr_ohos_DEFINED
