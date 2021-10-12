// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Framework adapted to ACE.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#ifndef SHELL_PLATFORM_OHOS_ACE_DISPLAY_MANAGER_JNI_H_
#define SHELL_PLATFORM_OHOS_ACE_DISPLAY_MANAGER_JNI_H_

#include <jni.h>
#include "flutter/fml/macros.h"
#include "flutter/fml/platform/android/jni_util.h"

namespace flutter {

class AceDisplayManagerJni final {
public:
    static bool Register(JNIEnv* env);
    static fml::jni::ScopedJavaGlobalRef<jclass>* GetClass();

    FML_DISALLOW_COPY_AND_ASSIGN(AceDisplayManagerJni);
};

} // namespace flutter

#endif // SHELL_PLATFORM_OHOS_ACE_DISPLAY_MANAGER_JNI_H_
