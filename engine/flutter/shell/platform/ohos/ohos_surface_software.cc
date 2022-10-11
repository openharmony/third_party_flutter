// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.4.30 platform view adapt ohos.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#include "flutter/shell/platform/ohos/ohos_surface_software.h"

#include <memory>
#include <vector>

#include "flutter/fml/logging.h"
#include "flutter/fml/trace_event.h"

#include "display_type.h"

namespace flutter {

namespace {

bool GetSkColorType(int32_t buffer_format,
                    SkColorType* color_type,
                    SkAlphaType* alpha_type)
{
    switch (buffer_format) {
        case PIXEL_FMT_RGB_565:
            *color_type = kRGB_565_SkColorType;
            *alpha_type = kOpaque_SkAlphaType;
            return true;
        case PIXEL_FMT_RGBA_8888:
            *color_type = kRGBA_8888_SkColorType;
            *alpha_type = kPremul_SkAlphaType;
            return true;
        default:
            return false;
    }
    return false;
}

}  // anonymous namespace

OhosSurfaceSoftware::OhosSurfaceSoftware()
{
    GetSkColorType(PIXEL_FMT_RGBA_8888, &target_color_type_, &target_alpha_type_);
}

bool OhosSurfaceSoftware::IsValid() const
{
    return true;
}

std::unique_ptr<Surface> OhosSurfaceSoftware::CreateGPUSurface()
{
    if (!IsValid()) {
        return nullptr;
    }

    auto surface = std::make_unique<GPUSurfaceSoftware>(this);

    if (!surface->IsValid()) {
        return nullptr;
    }

    return surface;
}

bool OhosSurfaceSoftware::OnScreenSurfaceResize(const SkISize& size)
{
    FML_LOG(INFO) << "OhosSurfaceSoftware::OnScreenSurfaceResize, software surface do noting";
    requestConfig_.width = size.fWidth;
    requestConfig_.height = size.fHeight;
    return true;
}

void OhosSurfaceSoftware::SetPlatformWindow(const OHOS::sptr<OHOS::Rosen::Window> &window)
{
    if (window == nullptr) {
        FML_LOG(ERROR) << "OhosSurfaceSoftware::SetPlatformWindow, window is nullptr";
        return;
    }
    window_ = window;
    auto surfaceNode = window->GetSurfaceNode();
    if (!surfaceNode) {
        FML_LOG(ERROR) << "surface node is null";
    }
    surface_ = surfaceNode->GetSurface();
    if (surface_ == nullptr) {
        FML_LOG(ERROR) << "OhosSurfaceSoftware::SetPlatformWindow, surface_ is nullptr";
        return;
    }
    requestConfig_ = {
        .width = surface_->GetDefaultWidth(),
        .height = surface_->GetDefaultHeight(),
        .strideAlignment = 0x8,
        .format = PIXEL_FMT_RGBA_8888,
        .usage = surface_->GetDefaultUsage(),
    };
    // Set buffer size to 5 for enough buffer
    surface_->SetQueueSize(5);
}

sk_sp<SkSurface> OhosSurfaceSoftware::AcquireBackingStore(
    const SkISize& size)
{
    TRACE_EVENT0("flutter", "OhosSurfaceSoftware::AcquireBackingStore");
    if (!IsValid()) {
        return nullptr;
    }

    if (sk_surface_ != nullptr && SkISize::Make(sk_surface_->width(), sk_surface_->height()) == size) {
        // The old and new surface sizes are the same. Nothing to do here.
        return sk_surface_;
    }

    SkImageInfo image_info = SkImageInfo::Make(
        size.fWidth, size.fHeight, target_color_type_, target_alpha_type_, SkColorSpace::MakeSRGB());

    sk_surface_ = SkSurface::MakeRaster(image_info);

    return sk_surface_;
}

bool OhosSurfaceSoftware::PresentBackingStore(
    sk_sp<SkSurface> backing_store)
{
    TRACE_EVENT0("flutter", "OhosSurfaceSoftware::PresentBackingStore");
    if (!IsValid() || backing_store == nullptr) {
        return false;
    }

    FML_LOG(INFO) << "OhosSurfaceSoftware peek pixels";
    SkPixmap pixmap;
    if (!backing_store->peekPixels(&pixmap)) {
        return false;
    }

    if (surface_ == nullptr) {
        FML_LOG(ERROR) << "OhosSurfaceSoftware surface is nullptr";
        return false;
    }
    int32_t pixelBase = 16;
    int32_t convertWidth = requestConfig_.width % pixelBase == 0 ? requestConfig_.width
        : (requestConfig_.width / pixelBase + 1) * pixelBase;
    int32_t alignment = 8;
    OHOS::BufferRequestConfig requestConfig = {
        .width = convertWidth,
        .height = requestConfig_.height,
        .strideAlignment = alignment,
        .format = PIXEL_FMT_RGBA_8888,
        .usage = OHOS::BUFFER_USAGE_CPU_READ | OHOS::BUFFER_USAGE_CPU_WRITE | OHOS::BUFFER_USAGE_MEM_DMA,
        .timeout = 0,
    };
    OHOS::sptr<OHOS::SurfaceBuffer> surfaceBuffer;
    int32_t releaseFence;

    OHOS::SurfaceError ret = surface_->RequestBuffer(surfaceBuffer, releaseFence, requestConfig);
    if (ret != OHOS::SURFACE_ERROR_OK || surfaceBuffer == nullptr || surfaceBuffer->GetSize() == 0) {
        FML_LOG(ERROR) << "OhosSurfaceSoftware request surfaceBuffer fail";
        return false;
    }

    memset(surfaceBuffer->GetVirAddr(), 0, surfaceBuffer->GetSize());
    SurfaceDrawBuffer(requestConfig, surfaceBuffer, pixmap);
    SurfaceFlushBuffer(surfaceBuffer);

    return true;
}

void OhosSurfaceSoftware::SurfaceDrawBuffer(
    OHOS::BufferRequestConfig& requestConfig, OHOS::sptr<OHOS::SurfaceBuffer> surfaceBuffer, SkPixmap& pixmap)
{
    SkColorType color_type;
    SkAlphaType alpha_type;
    if (GetSkColorType(requestConfig.format, &color_type, &alpha_type)) {
        SkImageInfo native_image_info = SkImageInfo::Make(
            requestConfig.width, requestConfig.height, color_type, alpha_type);

        std::unique_ptr<SkCanvas> canvas = SkCanvas::MakeRasterDirect(
            native_image_info, surfaceBuffer->GetVirAddr(),
            surfaceBuffer->GetBufferHandle()->stride);

        if (canvas) {
            SkBitmap bitmap;
            if (bitmap.installPixels(pixmap)) {
                canvas->drawBitmapRect(bitmap, SkRect::MakeIWH(requestConfig.width, requestConfig.height), nullptr);
            }
        }
    }
}

void OhosSurfaceSoftware::SurfaceFlushBuffer(OHOS::sptr<OHOS::SurfaceBuffer> surfaceBuffer)
{
    FML_LOG(INFO) << "OhosSurfaceSoftware flush buffer";
    OHOS::BufferFlushConfig flushConfig = {
        .damage = {
            .x = 0,
            .y = 0,
            .w = requestConfig_.width,
            .h = requestConfig_.height,
        },
        .timestamp = 0
    };
    surface_->FlushBuffer(surfaceBuffer, -1, flushConfig);
}

ExternalViewEmbedder* OhosSurfaceSoftware::GetExternalViewEmbedder()
{
    return nullptr;
}

bool OhosSurfaceSoftware::ResourceContextMakeCurrent()
{
    // implement in ohos surface gl
    return false;
}

bool OhosSurfaceSoftware::ResourceContextClearCurrent()
{
    // implement in ohos surface gl
    return false;
}

void OhosSurfaceSoftware::TeardownOnScreenContext()
{
    // implement in ohos surface gl
}

}  // namespace flutter
