// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.4.30 platform view adapt ohos.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#ifndef SHELL_PLATFORM_OHOS_PLATFORM_VIEW_OHOS_H_
#define SHELL_PLATFORM_OHOS_PLATFORM_VIEW_OHOS_H_

#include "flutter/shell/common/platform_view.h"
#include "flutter/shell/platform/ohos/ohos_surface.h"

namespace flutter {

class PlatformViewOhos final : public PlatformView {
public:
    PlatformViewOhos(PlatformView::Delegate& delegate, flutter::TaskRunners task_runners, bool use_software_rendering);

    void NotifyCreated(const ::OHOS::sptr<::OHOS::Rosen::Window> &window);
    void NotifyChanged(const SkISize& size);
    std::unique_ptr<Surface> CreateRenderingSurface();
    std::unique_ptr<VsyncWaiter> CreateVSyncWaiter(int32_t platform);

private:
    std::shared_ptr<OhosSurface> surface_;
    FML_DISALLOW_COPY_AND_ASSIGN(PlatformViewOhos);
};

}  // namespace flutter

#endif  // SHELL_PLATFORM_OHOS_PLATFORM_VIEW_OHOS_H_
