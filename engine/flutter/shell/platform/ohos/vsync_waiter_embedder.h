// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Framework adapted to ACE.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#ifndef SHELL_PLATFORM_OHOS_VSYNC_WAITER_EMBEDDER_H_
#define SHELL_PLATFORM_OHOS_VSYNC_WAITER_EMBEDDER_H_

#include <memory>

#include "flutter/fml/macros.h"
#include "flutter/shell/common/vsync_waiter.h"

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

    float fps_ = kUnknownRefreshRateFPS;

    int64_t refreshPeriod_ = 0;

    int64_t lastTimestamp_ = 0;

    FML_DISALLOW_COPY_AND_ASSIGN(VsyncWaiterEmbedder);
};

} // namespace flutter

#endif // SHELL_PLATFORM_OHOS_VSYNC_WAITER_OHOS_H_
