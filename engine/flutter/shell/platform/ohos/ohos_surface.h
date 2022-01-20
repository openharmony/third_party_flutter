// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.4.30 platform view adapt ohos.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#ifndef FLUTTER_SHELL_PLATFORM_OHOS_OHOS_SURFACE_H_
#define FLUTTER_SHELL_PLATFORM_OHOS_OHOS_SURFACE_H_

#include <memory>

#include "flutter/fml/macros.h"
#include "flutter/shell/common/surface.h"
#include "wm/window.h"
#include <ui/rs_surface_node.h>

namespace flutter {

class OhosSurface {
public:
    OhosSurface() = default;

    virtual ~OhosSurface() = default;

    virtual bool IsValid() const = 0;

    virtual std::unique_ptr<Surface> CreateGPUSurface() = 0;

    virtual bool OnScreenSurfaceResize(const SkISize& size) = 0;

    virtual void SetPlatformWindow(const ::OHOS::sptr<::OHOS::Rosen::Window> &window) = 0;

    virtual bool ResourceContextMakeCurrent() = 0;

    virtual bool ResourceContextClearCurrent() = 0;

    virtual void TeardownOnScreenContext() = 0;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_OHOS_OHOS_SURFACE_H_
