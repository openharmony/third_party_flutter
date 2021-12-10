// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.4.30 platform view adapt ohos.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#ifndef FLUTTER_SHELL_PLATFORM_OHOS_OHOS_SURFACE_GL_H_
#define FLUTTER_SHELL_PLATFORM_OHOS_OHOS_SURFACE_GL_H_

#include <memory>

#include "flutter/fml/macros.h"
#include "flutter/shell/gpu/gpu_surface_gl.h"
#include "flutter/shell/platform/ohos/ohos_surface.h"

#include "egl_surface.h"

namespace flutter {
class OhosSurfaceGL final : public GPUSurfaceGLDelegate, public OhosSurface {
public:
    OhosSurfaceGL();

    ~OhosSurfaceGL() override = default;

    // |OhosSurface|
    bool IsValid() const override;

    // |OhosSurface|
    std::unique_ptr<Surface> CreateGPUSurface() override;

    // |OhosSurface|
    void SetPlatformWindow(const ::OHOS::sptr<OHOS::Rosen::Window>& window) override;

    // |OhosSurface|
    bool OnScreenSurfaceResize(const SkISize& size) override;

    // |OhosSurface|
    void TeardownOnScreenContext() override;

    // |OhosSurface|
    bool ResourceContextMakeCurrent() override;

    // |OhosSurface|
    bool ResourceContextClearCurrent() override;

    // |GPUSurfaceGLDelegate|
    bool GLContextPresent() override;

    // |GPUSurfaceGLDelegate|
    intptr_t GLContextFBO() const override;

    // |GPUSurfaceGLDelegate|
    bool GLContextFBOResetAfterPresent() const override;

    // |GPUSurfaceGLDelegate|
    bool UseOffscreenSurface() const override;

    // |GPUSurfaceGLDelegate|
    SkMatrix GLContextSurfaceTransformation() const override;

    // |GPUSurfaceGLDelegate|
    bool GLContextMakeCurrent() override;

    // |GPUSurfaceGLDelegate|
    bool GLContextClearCurrent() override;

    // |GPUSurfaceGLDelegate|
    ExternalViewEmbedder* GetExternalViewEmbedder() override;

private:
    OHOS::sptr<OHOS::Rosen::Window> window_;
    OHOS::sptr<OHOS::EglRenderSurface> eglRenderSurface_;
    EGLDisplay eglDisplay_;
    EGLContext eglContext_;
    EGLSurface eglSurface_;
    bool valid_ = false;

    bool InitRenderSurface();

    FML_DISALLOW_COPY_AND_ASSIGN(OhosSurfaceGL);
};
}  // namespace flutter
#endif  // FLUTTER_SHELL_PLATFORM_OHOS_OHOS_SURFACE_GL_H_
