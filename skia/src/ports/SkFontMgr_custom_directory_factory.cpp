/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontMgr.h"
#include "include/ports/SkFontMgr_directory.h"

#ifndef SK_FONT_FILE_PREFIX
#    define SK_FONT_FILE_PREFIX "/usr/share/fonts/"
#endif

#ifdef SK_BUILD_FONT_MGR_FOR_PREVIEW_LINUX
SK_API sk_sp<SkFontMgr> SkFontMgr_New_OHOS(const char* path);
#endif

sk_sp<SkFontMgr> SkFontMgr::Factory() {
#if defined(SK_BUILD_FONT_MGR_FOR_PREVIEW_LINUX)
    return SkFontMgr_New_OHOS(nullptr);
#else
    return SkFontMgr_New_Custom_Directory(SK_FONT_FILE_PREFIX);
#endif
}
