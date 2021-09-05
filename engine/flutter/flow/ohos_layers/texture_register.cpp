// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Increase the process used in native_view mode.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#include "flutter/flow/ohos_layers/texture_register.h"

#include "flutter/shell/platform/android/platform_view_android_jni.h"

namespace flutter::OHOS {

namespace {

bool CheckException(JNIEnv* env)
{
    if (env->ExceptionCheck() == JNI_FALSE) {
        return true;
    }

    jthrowable exception = env->ExceptionOccurred();
    env->ExceptionClear();
    FML_LOG(ERROR) << fml::jni::GetJavaExceptionInfo(env, exception);
    env->DeleteLocalRef(exception);
    return false;
}

static fml::jni::ScopedJavaGlobalRef<jclass>* g_layerTextureClass = nullptr;
static jmethodID g_setAlphaMethod = nullptr;

void SetTextureLayerAlpha(JNIEnv* env, jobject obj, jint alpha)
{
    env->CallVoidMethod(obj, g_setAlphaMethod, alpha);
    FML_CHECK(CheckException(env));
}

} // namespace

bool AceTextureLayer::Register(JNIEnv* env)
{
    g_layerTextureClass = new fml::jni::ScopedJavaGlobalRef<jclass>(
      env, env->FindClass("ohos/ace/capability/texture/AceLayerTexture"));
    if (g_layerTextureClass->is_null()) {
        FML_LOG(ERROR) << "Could not locate AceLayerTexture class";
        return false;
    }
    g_setAlphaMethod = env->GetMethodID(
        g_layerTextureClass->obj(), "setLayerAlpha", "(I)V");

    if (g_setAlphaMethod == nullptr) {
        FML_LOG(ERROR) << "Could not locate setLayerAlpha method";
        return false;
    }
    return true;
}

void AceTextureLayer::setAlpha(int32_t alpha)
{
    alpha_ = alpha;
    JNIEnv* env = fml::jni::AttachCurrentThread();
    fml::jni::ScopedJavaLocalRef<jobject> layer = layerTexture_.get(env);
    if (!layer.is_null()) {
        SetTextureLayerAlpha(env, layer.obj(), static_cast<jint>(alpha_));
    }
}

void TextureRegistry::RegisterTexture(int64_t id, long textureHandle,
    const fml::jni::JavaObjectWeakGlobalRef& layerTexture)
{
    if (mapping_.find(id) == mapping_.end()) {
        mapping_[id] = { id, textureHandle, layerTexture };
    } else {
        mapping_[id].id_ = id;
        mapping_[id].handle_ = textureHandle;
        mapping_[id].layerTexture_ = layerTexture;
    }
}

void TextureRegistry::RegisterNativeWindow(int64_t id, const void* nativeWindow)
{
    if (mapping_.find(id) == mapping_.end()) {
        mapping_[id] = {
            .nativeWindow_ = const_cast<void*>(nativeWindow)
        };
    } else {
        mapping_[id].nativeWindow_ = const_cast<void*>(nativeWindow);
    }
}

void TextureRegistry::UnregisterTexture(int64_t id)
{
    mapping_.erase(id);
}

const AceTextureLayer& TextureRegistry::GetTexture(int64_t id)
{
    return mapping_[id];
}

}
