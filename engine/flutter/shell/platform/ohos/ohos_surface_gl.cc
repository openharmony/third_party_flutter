// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.4.30 platform view adapt ohos.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#include "flutter/shell/platform/ohos/ohos_surface_gl.h"

#include <utility>

#include "flutter/fml/logging.h"
#include "flutter/fml/memory/ref_ptr.h"

namespace flutter {
OhosSurfaceGL::OhosSurfaceGL()
    : window_(nullptr),
      eglRenderSurface_(nullptr),
      eglDisplay_(nullptr),
      eglContext_(nullptr),
      eglSurface_(nullptr),
      width_(0),
      height_(0),
      valid_(false)
{
    FML_LOG(ERROR) << "OhosSurfaceGL Constructor";
}

bool OhosSurfaceGL::IsValid() const
{
    return valid_;
}

std::unique_ptr<Surface> OhosSurfaceGL::CreateGPUSurface()
{
    auto surface = std::make_unique<GPUSurfaceGL>(this, true);
    return surface->IsValid() ? std::move(surface) : nullptr;
}

void OhosSurfaceGL::SetPlatformWindow(const OHOS::sptr<OHOS::Window>& window)
{
    if (window == nullptr) {
        FML_LOG(ERROR) << "Ohos window is nullptr";
        return;
    }

    window_ = window;

    // init render surface
    if (!this->InitRenderSurface()) {
        FML_LOG(ERROR) << "Failed to init render surface.";
    }
}

bool OhosSurfaceGL::OnScreenSurfaceResize(const SkISize& size)
{
    if (!IsValid()) {
        FML_LOG(ERROR) << "OnScreenSurfaceResize surface invalid";
        return false;
    }

    width_ = eglRenderSurface_->GetDefaultWidth();
    height_ = eglRenderSurface_->GetDefaultHeight();

    return true;
}

void OhosSurfaceGL::TeardownOnScreenContext()
{
    if (!IsValid()) {
        FML_LOG(ERROR) << "TeardownOnScreenContext surface invalid";
        return;
    }

    if (eglGetCurrentContext() != eglContext_) {
        FML_LOG(INFO) << "Egl context is diff";
        return;
    }

    if (eglMakeCurrent(eglDisplay_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) != EGL_TRUE) {
        FML_LOG(ERROR) << "Could not clear the current context";
        return;
    }
}

bool OhosSurfaceGL::ResourceContextMakeCurrent()
{
    return false;
}

bool OhosSurfaceGL::ResourceContextClearCurrent()
{
    return false;
}

bool OhosSurfaceGL::GLContextPresent()
{
    if (!IsValid()) {
        FML_LOG(ERROR) << "GLContextPresent surface invalid";
        return false;
    }

    OHOS::Rect rect = {0, 0, width_, height_};
    OHOS::SurfaceError errorCode = eglRenderSurface_->SwapBuffers(rect);
    if (errorCode != OHOS::SURFACE_ERROR_OK) {
        FML_LOG(ERROR) << "Failed to  swap buffers. code is " << errorCode;
        return false;
    }

    return true;
}

intptr_t OhosSurfaceGL::GLContextFBO() const
{
    if (!IsValid()) {
        FML_LOG(ERROR) << "GLContextFBO surface invalid";
        return 0;
    }

    return static_cast<intptr_t>(eglRenderSurface_->GetEglFbo());
}

bool OhosSurfaceGL::GLContextMakeCurrent()
{
    if (!IsValid()) {
        FML_LOG(ERROR) << "GLContextMakeCurrent surface invalid";
        return false;
    }

    // Avoid unused egl called.
    if (eglGetCurrentContext() == eglContext_) {
        FML_LOG(INFO) << "Egl context is same";
        return true;
    }

    if ((eglMakeCurrent(eglDisplay_, eglSurface_, eglSurface_, eglContext_) != EGL_TRUE)) {
        FML_LOG(ERROR) << "Could not make the context current. code is " << eglGetError();
        return false;
    }

    return true;
}

bool OhosSurfaceGL::GLContextClearCurrent()
{
    if (!IsValid()) {
        FML_LOG(ERROR) << "GLContextClearCurrent surface invalid";
        return false;
    }

    if (eglGetCurrentContext() != eglContext_) {
        FML_LOG(INFO) << "Egl context is diff";
        return true;
    }

    if (eglMakeCurrent(eglDisplay_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) != EGL_TRUE) {
        FML_LOG(ERROR) << "Could not clear the current context. code is " << eglGetError();
        return false;
    }

    return true;
}

ExternalViewEmbedder* OhosSurfaceGL::GetExternalViewEmbedder()
{
    return nullptr;
}

bool OhosSurfaceGL::InitRenderSurface()
{
    if (!window_) {
        FML_LOG(ERROR) << "window_ is nullptr";
        return false;
    }

    OHOS::sptr<OHOS::IBufferProducer> bufferProducer = window_->GetProducer();
    if (bufferProducer == nullptr) {
        FML_LOG(ERROR) << "bufferProducer is nullptr";
        return false;
    }

    eglRenderSurface_ = OHOS::EglRenderSurface::CreateEglRenderSurfaceAsProducer(bufferProducer);
    if (eglRenderSurface_ == nullptr) {
        FML_LOG(ERROR) << "eglRenderSurface_ is nullptr";
        return false;
    }

    OHOS::SurfaceError errorCode = eglRenderSurface_->InitContext();
    if (errorCode != OHOS::SURFACE_ERROR_OK) {
        FML_LOG(ERROR) << "Failed to init render context. code is " << errorCode;
        return false;
    }

    if ((eglGetCurrentContext() == eglContext_) &&
        (eglMakeCurrent(eglDisplay_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) != EGL_TRUE)) {
        FML_LOG(ERROR) << "Could not clear the current context. code is " << eglGetError();
    }

    eglDisplay_ = eglRenderSurface_->GetEglDisplay();
    if (eglDisplay_ == EGL_NO_DISPLAY) {
        FML_LOG(ERROR) << "EglDisplay_ is nullptr";
        return false;
    }

    eglContext_ = eglRenderSurface_->GetEglContext();
    if (eglContext_ == EGL_NO_CONTEXT) {
        FML_LOG(ERROR) << "EglContext_ is nullptr";
        return false;
    }

    // surface default EGL_NO_SURFACE
    eglSurface_ = eglRenderSurface_->GetEglSurface();
    width_ = eglRenderSurface_->GetDefaultWidth();
    height_ = eglRenderSurface_->GetDefaultHeight();
    valid_ = true;

    FML_LOG(INFO) << "EglRenderSurface width " << width_ << " height " << height_;

    return true;
}
}  // namespace flutter