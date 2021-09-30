// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.4.30 platform view adapt ohos.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#include "flutter/shell/platform/ohos/platform_view_ohos.h"

#include <memory>
#include <utility>

#include "flutter/common/settings.h"
#ifdef ACE_ENABLE_GPU
#include "flutter/shell/platform/ohos/ohos_surface_gl.h"
#endif
#include "flutter/shell/platform/ohos/ohos_surface_software.h"
#include "flutter/shell/platform/ohos/vsync_waiter_embedder.h"

namespace flutter {

PlatformViewOhos::PlatformViewOhos(
    PlatformView::Delegate& delegate,
    flutter::TaskRunners task_runners,
    bool use_software_rendering)
    : PlatformView(delegate, std::move(task_runners))
{
#ifdef ACE_ENABLE_GPU
    surface_ = std::make_shared<OhosSurfaceGL>();
#else
    if (use_software_rendering) {
        surface_ = std::make_shared<OhosSurfaceSoftware>();
    }
#endif
}

void PlatformViewOhos::NotifyCreated(const ::OHOS::sptr<::OHOS::Window> &window)
{
    if (surface_) {
#ifdef ACE_ENABLE_GPU
        fml::AutoResetWaitableEvent latch;
        fml::TaskRunner::RunNowOrPostTask(
            task_runners_.GetGPUTaskRunner(),
            [&latch, surface = surface_.get(), &window]() mutable {
                surface->SetPlatformWindow(window);
                latch.Signal();
            });
        latch.Wait();
#else
        surface_->SetPlatformWindow(window);
#endif
    }

    PlatformView::NotifyCreated();
}

void PlatformViewOhos::NotifyChanged(const SkISize& size)
{
    if (surface_) {
#ifdef ACE_ENABLE_GPU
        fml::AutoResetWaitableEvent latch;
        fml::TaskRunner::RunNowOrPostTask(
            task_runners_.GetGPUTaskRunner(),
            [&latch, surface = surface_.get(), &size]() mutable {
                surface->OnScreenSurfaceResize(size);
                latch.Signal();
            });
        latch.Wait();
#else
        surface_->OnScreenSurfaceResize(size);
#endif
    }
}

std::unique_ptr<Surface> PlatformViewOhos::CreateRenderingSurface()
{
    if (!surface_) {
        return nullptr;
    }
    return surface_->CreateGPUSurface();
}

std::unique_ptr<VsyncWaiter> PlatformViewOhos::CreateVSyncWaiter(int32_t platform)
{
    return VsyncWaiterEmbedder::Create(task_runners_);
}

}  // namespace flutter
