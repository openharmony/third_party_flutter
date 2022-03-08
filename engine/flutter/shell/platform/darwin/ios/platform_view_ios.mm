// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/darwin/ios/platform_view_ios.h"

#import <QuartzCore/CAEAGLLayer.h>

#include <utility>

#include "flutter/common/task_runners.h"
#include "flutter/fml/synchronization/waitable_event.h"
#include "flutter/fml/trace_event.h"
#include "flutter/shell/common/shell_io_manager.h"
#include "flutter/shell/gpu/gpu_surface_gl_delegate.h"
#include "flutter/shell/platform/darwin/ios/framework/Source/FlutterViewController_Internal.h"
#include "flutter/shell/platform/darwin/ios/framework/Source/vsync_waiter_ios.h"
#include "flutter/shell/platform/darwin/ios/ios_external_texture_gl.h"

namespace flutter {

PlatformViewIOS::PlatformViewIOS(PlatformView::Delegate& delegate,
                                 flutter::TaskRunners task_runners)
    : PlatformView(delegate, std::move(task_runners)) {
#if !TARGET_IPHONE_SIMULATOR
  gl_context_ = std::make_shared<IOSGLContext>();
#endif  // !TARGET_IPHONE_SIMULATOR
}

PlatformViewIOS::~PlatformViewIOS() = default;

PlatformMessageRouter& PlatformViewIOS::GetPlatformMessageRouter() {
  return platform_message_router_;
}

// |PlatformView|
void PlatformViewIOS::HandlePlatformMessage(fml::RefPtr<flutter::PlatformMessage> message) {
  platform_message_router_.HandlePlatformMessage(std::move(message));
}

fml::WeakPtr<FlutterViewController> PlatformViewIOS::GetOwnerViewController() const {
  return owner_controller_;
}

void PlatformViewIOS::SetOwnerViewController(fml::WeakPtr<FlutterViewController> owner_controller) {
  if (ios_surface_ || !owner_controller) {
    NotifyDestroyed();
    ios_surface_.reset();
  }
  owner_controller_ = owner_controller;
  if (owner_controller_) {
    ios_surface_ =
        [static_cast<FlutterView*>(owner_controller.get().view) createSurface:gl_context_];
    FML_DCHECK(ios_surface_ != nullptr);
  }
}

void PlatformViewIOS::RegisterExternalTexture(int64_t texture_id,
                                              NSObject<FlutterTexture>* texture) {
  RegisterTexture(std::make_shared<IOSExternalTextureGL>(texture_id, texture));
}

// |PlatformView|
std::unique_ptr<Surface> PlatformViewIOS::CreateRenderingSurface() {
  if (!ios_surface_) {
    FML_DLOG(INFO) << "Could not CreateRenderingSurface, this PlatformViewIOS "
                      "has no ViewController.";
    return nullptr;
  }
  return ios_surface_->CreateGPUSurface();
}

// |PlatformView|
sk_sp<GrContext> PlatformViewIOS::CreateResourceContext() const {
  if (!gl_context_ || !gl_context_->ResourceMakeCurrent()) {
    FML_DLOG(INFO) << "Could not make resource context current on IO thread. "
                      "Async texture uploads will be disabled. On Simulators, "
                      "this is expected.";
    return nullptr;
  }

  return ShellIOManager::CreateCompatibleResourceLoadingContext(
      GrBackend::kOpenGL_GrBackend, GPUSurfaceGLDelegate::GetDefaultPlatformGLInterface());
}

// |PlatformView|
std::unique_ptr<VsyncWaiter> PlatformViewIOS::CreateVSyncWaiter(int32_t platform) {
//    if (platform == static_cast<int32_t>(AcePlatform::ACE_PLATFORM_IOS)) {
        return std::make_unique<VsyncWaiterIOS>(task_runners_);
//    }
//
//    return nullptr;
}

void PlatformViewIOS::OnPreEngineRestart() const {
  if (!owner_controller_) {
    return;
  }
  [owner_controller_.get() platformViewsController] -> Reset();
}

}  // namespace flutter
