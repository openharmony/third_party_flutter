// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Increase the process used in native_view mode.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#ifndef FLUTTER_FLOW_OHOS_LAYERS_TEXTURE_REGISTER_H
#define FLUTTER_FLOW_OHOS_LAYERS_TEXTURE_REGISTER_H

#include <unordered_map>

#include "flutter/fml/macros.h"
#include "flutter/fml/platform/android/jni_util.h"
#include "flutter/fml/platform/android/jni_weak_ref.h"

namespace flutter::OHOS {

struct AceTextureLayer {
    void setAlpha(int32_t alpha);
    static bool Register(JNIEnv* env);

    int64_t id_ = -1;
    long handle_ = 0;
    fml::jni::JavaObjectWeakGlobalRef layerTexture_;
    void* nativeWindow_ = nullptr;
    int32_t alpha_ = 255;
};

class TextureRegistry {
public:
    TextureRegistry() = default;
    ~TextureRegistry() = default;

    // Called from Platform thread.
    void RegisterTexture(int64_t id, long textureHandle, const fml::jni::JavaObjectWeakGlobalRef& layerTexture);

    // Called from Platform thread.
    void RegisterNativeWindow(int64_t id, const void* nativeWindow);

    // Called from Platform thread.
    void UnregisterTexture(int64_t id);

    // Called from Platform thread.
    const AceTextureLayer& GetTexture(int64_t id);

private:
    std::unordered_map<int64_t, AceTextureLayer> mapping_;

    FML_DISALLOW_COPY_AND_ASSIGN(TextureRegistry);
};

}  // namespace flutter::OHOS

#endif  // FLUTTER_FLOW_OHOS_LAYERS_TEXTURE_REGISTER_H
