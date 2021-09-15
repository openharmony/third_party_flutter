// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Framework adapted to ACE.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#include <cmath>
#include <utility>

#include "flutter/common/task_runners.h"
#include "flutter/shell/platform/ohos/vsync_waiter_embedder.h"
#include "vsync_helper.h"

namespace flutter {

namespace {

constexpr float ONE_SECOND_IN_NANO = 1000000000.0f;
constexpr float TOLERATE_PERCENT = 0.96f;

} // namespace

std::unique_ptr<VsyncWaiter> VsyncWaiterEmbedder::Create(flutter::TaskRunners task_runners)
{
    return std::make_unique<VsyncWaiterEmbedder>(task_runners);
}

VsyncWaiterEmbedder::VsyncWaiterEmbedder(flutter::TaskRunners task_runners) : VsyncWaiter(std::move(task_runners))
{
    fps_ = GetDisplayRefreshRate();
    if (fps_ != kUnknownRefreshRateFPS) {
        refreshPeriod_ = static_cast<int64_t>(ONE_SECOND_IN_NANO / fps_);
    }
}

VsyncWaiterEmbedder::~VsyncWaiterEmbedder() = default;

typedef struct CallbackInfo_ {
    int64_t refreshPeriod_;
    float fps_;
    std::weak_ptr<VsyncWaiter> weak_base_;
} CallbackInfo;

void VsyncWaiterEmbedder::VSyncCallback(int64_t nanoTimestamp, void* userdata)
{
    CallbackInfo* info = (CallbackInfo*) userdata;
    int64_t refreshPeriod = info->refreshPeriod_;
    float fps = info->fps_;
    std::weak_ptr<VsyncWaiter> weak_base = info->weak_base_;

    auto shared_base = weak_base.lock();
    auto shared_this = static_cast<VsyncWaiterEmbedder*>(shared_base.get());
    if (shared_base && fps != kUnknownRefreshRateFPS && shared_this) {
        if (nanoTimestamp < shared_this->lastTimestamp_ + refreshPeriod * TOLERATE_PERCENT) {
            nanoTimestamp = shared_this->lastTimestamp_ + refreshPeriod;
        }
        shared_this->lastTimestamp_ = nanoTimestamp;

        auto frame_time = fml::TimePoint::FromEpochDelta(fml::TimeDelta::FromNanoseconds(nanoTimestamp));
        auto target_time =
            fml::TimePoint::FromEpochDelta(fml::TimeDelta::FromNanoseconds(nanoTimestamp + refreshPeriod));
        shared_base->FireCallback(frame_time, target_time);
    }
    delete info;
}

// |VsyncWaiter|
void VsyncWaiterEmbedder::AwaitVSync()
{
    if (fps_ == kUnknownRefreshRateFPS) {
        fps_ = GetDisplayRefreshRate();
        if (fps_ != kUnknownRefreshRateFPS) {
            refreshPeriod_ = static_cast<int64_t>(ONE_SECOND_IN_NANO / fps_);
        }
    }
    std::weak_ptr<VsyncWaiter> weak_base(shared_from_this());
    CallbackInfo* info = new CallbackInfo{ refreshPeriod_, fps_, weak_base };
    task_runners_.GetPlatformTaskRunner()->PostTask([info]() {
        struct OHOS::FrameCallback cb = {
            .timestamp_ = 0,
            .userdata_ = info,
            .callback_ = VSyncCallback,
        };
        OHOS::VsyncHelper::Current()->RequestFrameCallback(cb);
    });
}

float VsyncWaiterEmbedder::GetDisplayRefreshRate() const
{
    return 60.0f;
}

} // namespace flutter
