/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 * 2021.2.10 SkFontMgr on ohos.
 *           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.
 */

#include "src/ports/SkFontMgr_ohos.h"

SkFontMgr_OHOS::BuildFamilyMapCallback SkFontMgr_OHOS::buildFamilyMapCallback;

SkFontMgr_OHOS::SkFontMgr_OHOS()
{
    this->buildNameToFamilyMap();
    this->findDefaultStyleSet();
}

int SkFontMgr_OHOS::onCountFamilies() const
{
    return fNameToFamilyMap.count();
}

void SkFontMgr_OHOS::onGetFamilyName(int index, SkString* familyName) const
{
    if (index < 0 || fNameToFamilyMap.count() <= index) {
        familyName->reset();
        return;
    }
    familyName->set(fNameToFamilyMap[index].name);
}

SkFontStyleSet* SkFontMgr_OHOS::onCreateStyleSet(int index) const
{
    if (index < 0 || fNameToFamilyMap.count() <= index) {
        return nullptr;
    }
    return SkRef(fNameToFamilyMap[index].styleSet);
}

SkFontStyleSet* SkFontMgr_OHOS::onMatchFamily(const char familyName[]) const
{
    if (!familyName) {
        return nullptr;
    }
    std::string familyNameStr(familyName);
    std::transform(familyNameStr.begin(), familyNameStr.end(), familyNameStr.begin(),
        [](unsigned char c) -> unsigned char { return std::tolower(c); });

    std::string aliasTo;
    const auto& iter = fAliasMap.find(familyNameStr);
    if (iter != fAliasMap.end()) {
        aliasTo = iter->second.toName;
    }
    for (int i = 0; i < fNameToFamilyMap.count(); ++i) {
        if (fNameToFamilyMap[i].name.equals(familyNameStr.c_str())) {
            return SkRef(fNameToFamilyMap[i].styleSet);
        }
        if (!aliasTo.empty() && fNameToFamilyMap[i].name.equals(aliasTo.c_str())) {
            return SkRef(fNameToFamilyMap[i].styleSet);
        }
    }

    for (int i = 0; i < fFallbackNameToFamilyMap.count(); ++i) {
        if (fFallbackNameToFamilyMap[i].name.equals(familyNameStr.c_str())) {
            return SkRef(fFallbackNameToFamilyMap[i].styleSet);
        }
    }

    for (int i = 0; i < fFallbackNameToFamilyMap.count(); ++i) {
        const auto& styleSet = fFallbackNameToFamilyMap[i].styleSet;
        if (styleSet) {
            const auto& host = styleSet->fStylesHost;
            for (int j = 0; j < host.count(); ++j) {
                if (host[j]->getFamilyName().equals(familyNameStr.c_str())) {
                    return SkRef(fFallbackNameToFamilyMap[i].styleSet);
                }
            }
        }
    }
    return nullptr;
}

SkTypeface* SkFontMgr_OHOS::onMatchFamilyStyle(const char familyName[], const SkFontStyle& style) const
{
    sk_sp<SkFontStyleSet> sset(this->matchFamily(familyName));
    return sset->matchStyle(style);
}

SkTypeface* SkFontMgr_OHOS::onMatchFaceStyle(const SkTypeface* typeface, const SkFontStyle& style) const
{
    for (int i = 0; i < fStyleSets.count(); ++i) {
        for (int j = 0; j < fStyleSets[i]->fStyles.count(); ++j) {
            if (fStyleSets[i]->fStyles[j].get() == typeface) {
                return fStyleSets[i]->matchStyle(style);
            }
        }
    }
    return nullptr;
}

sk_sp<SkTypeface> SkFontMgr_OHOS::find_family_style_character(const SkString& familyName,
    const SkTArray<NameToFamily, true>& fallbackNameToFamilyMap, const SkFontStyle& style, bool elegant,
    const SkString& langTag, SkUnichar character)
{
    for (int i = 0; i < fallbackNameToFamilyMap.count(); ++i) {
        SkFontStyleSet_OHOS* family = fallbackNameToFamilyMap[i].styleSet;
        // TODO: process fallbackFor
        SkTypeface* face = family->matchStyle(style);
        if (!face) {
            SkDEBUGF("face is null");
            continue;
        }

        if (!family->matchLanguage(langTag)) {
            continue;
        }

        if (family->haveVariant(kElegant_FontVariant) != elegant) {
            continue;
        }

        if (face->unicharToGlyph(character) != 0) {
            return sk_sp<SkTypeface>(face);
        }
    }
    return nullptr;
}

SkString SkFontMgr_OHOS::find_family_style_character_ohos(const SkString& familyName,
    const SkTArray<NameToFamily, true>& fallbackNameToFamilyMap, const SkFontStyle& style, bool elegant,
    const SkString& langTag, SkUnichar character)
{
    SkString matchingName = findFromCache(familyName, style, elegant, langTag, character);
    if (!matchingName.isEmpty()) {
        return matchingName;
    }

    for (int i = 0; i < fallbackNameToFamilyMap.count(); ++i) {
        SkFontStyleSet_OHOS* family = fallbackNameToFamilyMap[i].styleSet;
        // TODO: process fallbackFor
        SkTypeface* face = family->matchStyle(style);
        if (!face) {
            SkDEBUGF("face is null");
            continue;
        }

        if (!family->matchLanguage(langTag)) {
            continue;
        }

        if (family->haveVariant(kElegant_FontVariant) != elegant) {
            continue;
        }

        if (face->unicharToGlyph(character) != 0) {
            addToCache(&(fallbackNameToFamilyMap[i]));
            return fallbackNameToFamilyMap[i].name;
        }
    }
    return SkString();
}

SkString SkFontMgr_OHOS::find_family_style_character_hwfont(
    const SkString &familyName,
    const SkTArray<NameToFamily, true> &fallbackNameToFamilyMap,
    const SkFontStyle &style, bool elegant, const SkString &langTag,
    SkUnichar character) {
    SkString matchingName =
        findFromHwFontCache(familyName, style, elegant, langTag, character);
    if (!matchingName.isEmpty()) {
        return matchingName;
    }

    // First find in HwThemeFont.
    for (int i = 0; i < fallbackNameToFamilyMap.count(); ++i) {
        SkFontStyleSet_OHOS *family = fallbackNameToFamilyMap[i].styleSet;
        if (family->getHwFontFamilyType() > 0) {
            SkTypeface *face = family->matchStyle(style);
            if (!face) {
                SkDEBUGF("face is null");
                continue;
            }

            if (!family->matchLanguage(langTag)) {
               continue;
            }

            if (family->haveVariant(kElegant_FontVariant) != elegant) {
                continue;
            }

            if (face->unicharToGlyph(character) != 0) {
                addToHwFontCache(&(fallbackNameToFamilyMap[i]));
                return fallbackNameToFamilyMap[i].name;
            }
        }
    }
    return SkString();
}

SkString SkFontMgr_OHOS::findFromCache(const SkString& familyName, const SkFontStyle& style, bool elegant,
                           const SkString& langTag, SkUnichar character)
{
    std::lock_guard<std::mutex> lock(mutexCache);
    for (int i = 0; i < familyMapCache.count(); ++i) {
        SkFontStyleSet_OHOS* family = familyMapCache[i]->styleSet;
        // TODO: process fallbackFor
        SkTypeface* face = family->matchStyle(style);
        if (!face) {
            SkDEBUGF("face is null");
            continue;
        }

        if (!family->matchLanguage(langTag)) {
            continue;
        }

        if (family->haveVariant(kElegant_FontVariant) != elegant) {
            continue;
        }

        if (face->unicharToGlyph(character) != 0) {
            return familyMapCache[i]->name;
        }
    }
    return SkString();
}

void SkFontMgr_OHOS::addToCache(const NameToFamily* item)
{
    std::lock_guard<std::mutex> lock(mutexCache);
    for (int i = 0; i < familyMapCache.count(); ++i) {
        if (item == familyMapCache[i]) {
            return;
        }
    }
    familyMapCache.emplace_back(item);
}

SkString SkFontMgr_OHOS::findFromHwFontCache(const SkString& familyName, const SkFontStyle& style, bool elegant,
                           const SkString& langTag, SkUnichar character)
{
    std::lock_guard<std::mutex> lock(mutexCache);
    for (int i = 0; i < hwFontFamilyMapCache.count(); ++i) {
        SkFontStyleSet_OHOS* family = hwFontFamilyMapCache[i]->styleSet;
        // TODO: process fallbackFor
        SkTypeface* face = family->matchStyle(style);
        if (!face) {
            SkDEBUGF("face is null");
            continue;
        }

        if (!family->matchLanguage(langTag)) {
            continue;
        }

        if (family->haveVariant(kElegant_FontVariant) != elegant) {
            continue;
        }

        if (face->unicharToGlyph(character) != 0) {
            return hwFontFamilyMapCache[i]->name;
        }
    }
    return SkString();
}

void SkFontMgr_OHOS::addToHwFontCache(const NameToFamily* item)
{
    std::lock_guard<std::mutex> lock(mutexCache);
    for (int i = 0; i < hwFontFamilyMapCache.count(); ++i) {
        if (item == hwFontFamilyMapCache[i]) {
            return;
        }
    }
    hwFontFamilyMapCache.emplace_back(item);
}

SkTypeface* SkFontMgr_OHOS::onMatchFamilyStyleCharacter(
    const char familyName[], const SkFontStyle& style, const char* bcp47[], int bcp47Count, SkUnichar character) const
{
    SkString familyNameString(familyName);
    for (const SkString& currentFamilyName : { familyNameString, SkString() }) {
        // The first time match anything elegant, second time anything not elegant.
        for (int elegant = 2; elegant-- > 0;) {
            for (int bcp47Index = bcp47Count; bcp47Index-- > 0;) {
                SkLanguage lang(bcp47[bcp47Index]);
                while (!lang.getTag().isEmpty()) {
                    sk_sp<SkTypeface> matchingTypeface = find_family_style_character(currentFamilyName,
                        fFallbackNameToFamilyMap, style, SkToBool(elegant), lang.getTag(), character);
                    if (matchingTypeface) {
                        return matchingTypeface.release();
                    }
                    lang = lang.getParent();
                }
            }
            sk_sp<SkTypeface> matchingTypeface = find_family_style_character(
                currentFamilyName, fFallbackNameToFamilyMap, style, SkToBool(elegant), SkString(), character);
            if (matchingTypeface) {
                return matchingTypeface.release();
            }
        }
    }
    return nullptr;
}

SkString SkFontMgr_OHOS::onMatchFamilyStyleCharacterOHOS(
    const char familyName[], const SkFontStyle& style, const char* bcp47[], int bcp47Count, SkUnichar character)
{
    SkString familyNameString(familyName);
    for (const SkString& currentFamilyName : { familyNameString, SkString() }) {
        // The first time match anything elegant, second time anything not elegant.
        for (int bcp47Index = bcp47Count; bcp47Index-- > 0;) {
            SkLanguage lang(bcp47[bcp47Index]);
            while (!lang.getTag().isEmpty()) {
                SkString matchingName = find_family_style_character_ohos(
                    currentFamilyName, fFallbackNameToFamilyMap, style, false, lang.getTag(), character);
                if (!matchingName.isEmpty()) {
                    return matchingName;
                }
                lang = lang.getParent();
            }
        }
        SkString matchingName = find_family_style_character_ohos(
            currentFamilyName, fFallbackNameToFamilyMap, style, false, SkString(), character);
        if (!matchingName.isEmpty()) {
            return matchingName;
        }
    }
    return SkString();
}

SkString SkFontMgr_OHOS::onMatchFamilyStyleCharacterHwFont(
    const char familyName[], const SkFontStyle& style, const char* bcp47[], int bcp47Count, SkUnichar character)
{
    SkString familyNameString(familyName);
    for (const SkString& currentFamilyName : { familyNameString, SkString() }) {
        // The first time match anything elegant, second time anything not elegant.
        for (int bcp47Index = bcp47Count; bcp47Index-- > 0;) {
            SkLanguage lang(bcp47[bcp47Index]);
            while (!lang.getTag().isEmpty()) {
                SkString matchingName = find_family_style_character_hwfont(
                    currentFamilyName, fFallbackNameToFamilyMap, style, false, lang.getTag(), character);
                if (!matchingName.isEmpty()) {
                    return matchingName;
                }
                lang = lang.getParent();
            }
        }
        SkString matchingName = find_family_style_character_hwfont(
            currentFamilyName, fFallbackNameToFamilyMap, style, false, SkString(), character);
        if (!matchingName.isEmpty()) {
            return matchingName;
        }
    }
    return SkString();
}

sk_sp<SkTypeface> SkFontMgr_OHOS::onMakeFromData(sk_sp<SkData> data, int ttcIndex) const
{
    return this->makeFromStream(std::unique_ptr<SkStreamAsset>(new SkMemoryStream(std::move(data))), ttcIndex);
}

sk_sp<SkTypeface> SkFontMgr_OHOS::onMakeFromFile(const char path[], int ttcIndex) const
{
    std::unique_ptr<SkStreamAsset> stream = SkStream::MakeFromFile(path);
    return stream.get() ? this->makeFromStream(std::move(stream), ttcIndex) : nullptr;
}

sk_sp<SkTypeface> SkFontMgr_OHOS::onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset> stream, int ttcIndex) const
{
    SkDEBUGF("onMakeFromStreamIndex not support");
    return nullptr;
}

sk_sp<SkTypeface> SkFontMgr_OHOS::onMakeFromStreamArgs(
    std::unique_ptr<SkStreamAsset> stream, const SkFontArguments& args) const
{
    SkDEBUGF("onMakeFromStreamArgs not support");
    return nullptr;
}

sk_sp<SkTypeface> SkFontMgr_OHOS::onMakeFromFontData(std::unique_ptr<SkFontData> data) const
{
    SkDEBUGF("onMakeFromFontData not support");
    return nullptr;
}

sk_sp<SkTypeface> SkFontMgr_OHOS::onLegacyMakeTypeface(const char familyName[], SkFontStyle style) const
{
    if (familyName) {
        return sk_sp<SkTypeface>(this->onMatchFamilyStyle(familyName, style));
    }
    return sk_sp<SkTypeface>(fDefaultStyleSet->matchStyle(style));
}

void SkFontMgr_OHOS::buildNameToFamilyMap()
{
    if (buildFamilyMapCallback) {
        buildFamilyMapCallback(fNameToFamilyMap, fFallbackNameToFamilyMap, fStyleSets, fAliasMap);
    }
}

void SkFontMgr_OHOS::findDefaultStyleSet()
{
    if (fStyleSets.empty()) {
        return;
    }

    static const char* defaultNames[] = { "sans-serif" };
    for (const char* defaultName : defaultNames) {
        fDefaultStyleSet.reset(this->onMatchFamily(defaultName));
        if (fDefaultStyleSet) {
            break;
        }
    }
    if (nullptr == fDefaultStyleSet) {
        fDefaultStyleSet = fStyleSets[0];
    }
    SkASSERT(fDefaultStyleSet);
}

sk_sp<SkFontMgr> SkFontMgr_New_OHOS()
{
    return sk_make_sp<SkFontMgr_OHOS>();
}
