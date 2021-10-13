// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Framework adapted to ACE.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#include "flutter/fml/logging.h"
#include "flutter/shell/common/ace_display_manager_jni.h"

namespace flutter {

static fml::jni::ScopedJavaGlobalRef<jclass>* g_vsync_waiter_class = nullptr;

// static
bool AceDisplayManagerJni::Register(JNIEnv* env)
{
    jclass clazz = env->FindClass("ohos/ace/AceDisplayManager");
    if (clazz == nullptr) {
        return false;
    }

    g_vsync_waiter_class = new fml::jni::ScopedJavaGlobalRef<jclass>(env, clazz);
    FML_CHECK(!g_vsync_waiter_class->is_null());
    return true;
}

fml::jni::ScopedJavaGlobalRef<jclass>* AceDisplayManagerJni::GetClass()
{
    return g_vsync_waiter_class;
}

} // namespace flutter
