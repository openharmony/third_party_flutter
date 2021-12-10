// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.4.30 platform view adapt ohos.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#ifndef FLUTTER_SHELL_PLATFORM_OHOS_OHOS_SURFACE_SOFTWARE_H_
#define FLUTTER_SHELL_PLATFORM_OHOS_OHOS_SURFACE_SOFTWARE_H_

#include "flutter/fml/macros.h"
#include "flutter/shell/gpu/gpu_surface_software.h"
#include "flutter/shell/platform/ohos/ohos_surface.h"

namespace flutter {

class OhosSurfaceSoftware final : public OhosSurface, public GPUSurfaceSoftwareDelegate {
public:
    OhosSurfaceSoftware();

    ~OhosSurfaceSoftware() override = default;

    bool IsValid() const override;

    std::unique_ptr<Surface> CreateGPUSurface() override;

    bool OnScreenSurfaceResize(const SkISize& size) override;

    void SetPlatformWindow(const OHOS::sptr<OHOS::Rosen::Window> &window) override;

    sk_sp<SkSurface> AcquireBackingStore(const SkISize& size) override;

    bool PresentBackingStore(sk_sp<SkSurface> backing_store) override;

    void SurfaceDrawBuffer(
        OHOS::BufferRequestConfig& requestConfig, OHOS::sptr<OHOS::SurfaceBuffer> surfaceBuffer, SkPixmap& pixmap);

    void SurfaceFlushBuffer(OHOS::sptr<OHOS::SurfaceBuffer>);

    ExternalViewEmbedder* GetExternalViewEmbedder() override;

    // |OhosSurface|
    virtual bool ResourceContextMakeCurrent() override;

    // |OhosSurface|
    virtual bool ResourceContextClearCurrent() override;

    // |OhosSurface|
    virtual void TeardownOnScreenContext() override;

private:
    sk_sp<SkSurface> sk_surface_;
    SkColorType target_color_type_;
    SkAlphaType target_alpha_type_;

    OHOS::sptr<OHOS::Rosen::Window> window_ = nullptr;
    OHOS::BufferRequestConfig requestConfig_;
    OHOS::sptr<OHOS::Surface> surface_ = nullptr;

    FML_DISALLOW_COPY_AND_ASSIGN(OhosSurfaceSoftware);
};
}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_OHOS_OHOS_SURFACE_SOFTWARE_H_
