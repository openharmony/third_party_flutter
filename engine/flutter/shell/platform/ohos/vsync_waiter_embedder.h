// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Framework adapted to ACE.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#ifndef SHELL_PLATFORM_OHOS_VSYNC_WAITER_OHOS_H_
#define SHELL_PLATFORM_OHOS_VSYNC_WAITER_OHOS_H_

#ifndef OHOS_STANDARD_SYSTEM
#include <jni.h>
#endif
#include <memory>

#ifndef OHOS_STANDARD_SYSTEM
#include "platform/vsync/agp_vsync_scheduler.h"
#endif

#include "flutter/fml/macros.h"
#include "flutter/shell/common/vsync_waiter.h"
#include "vsync_receiver.h"

namespace flutter {

class VsyncWaiterEmbedder final : public VsyncWaiter {
public:
    static std::unique_ptr<VsyncWaiter> Create(flutter::TaskRunners);
    static void VSyncCallback(int64_t nanoTimestamp, void* userdata);

    VsyncWaiterEmbedder(flutter::TaskRunners task_runners);

    ~VsyncWaiterEmbedder() override;

    float GetDisplayRefreshRate() const override;

private:
    // |VsyncWaiter|
    void AwaitVSync() override;

#ifndef OHOS_STANDARD_SYSTEM
    std::shared_ptr<::OHOS::AGP::VsyncScheduler> vsync_scheduler_;
#endif

    float fps_ = kUnknownRefreshRateFPS;

    int64_t refreshPeriod_ = 0;

    std::shared_ptr<OHOS::Rosen::VSyncReceiver> vsyncReceiver_ = nullptr;

    FML_DISALLOW_COPY_AND_ASSIGN(VsyncWaiterEmbedder);
};

} // namespace flutter

#endif // SHELL_PLATFORM_OHOS_VSYNC_WAITER_OHOS_H_
