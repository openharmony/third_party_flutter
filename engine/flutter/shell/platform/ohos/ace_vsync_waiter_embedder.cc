// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Framework adapted to ACE.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#include <cmath>
#include <utility>

#include "flutter/common/task_runners.h"
#include "flutter/shell/platform/ohos/vsync_waiter_embedder.h"
#ifdef OHOS_STANDARD_SYSTEM
#include "transaction/rs_interfaces.h"
#else
#include "flutter/fml/platform/android/jni_util.h"
#include "flutter/shell/common/ace_display_manager_jni.h"
#endif
namespace flutter {

namespace {

constexpr float ONE_SECOND_IN_NANO = 1000000000.0f;

} // namespace

std::unique_ptr<VsyncWaiter> VsyncWaiterEmbedder::Create(flutter::TaskRunners task_runners)
{
    return std::make_unique<VsyncWaiterEmbedder>(task_runners);
}

VsyncWaiterEmbedder::VsyncWaiterEmbedder(flutter::TaskRunners task_runners) : VsyncWaiter(std::move(task_runners))
{
#ifndef OHOS_STANDARD_SYSTEM
    vsync_scheduler_ = OHOS::AGP::VsyncScheduler::Create();
#endif
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
#ifdef OHOS_STANDARD_SYSTEM
    if (vsyncReceiver_) {
        auto& rsClient = OHOS::Rosen::RSInterfaces::GetInstance();
        vsyncReceiver_ = rsClient.CreateVSyncReceiver("ACE");
        vsyncReceiver_->Init();
    }
    std::weak_ptr<OHOS::Rosen::VSyncReceiver> weakReceiver = vsyncReceiver_;
    task_runners_.GetPlatformTaskRunner()->PostTask([weakReceiver, info]() {
        OHOS::Rosen::VSyncReceiver::FrameCallback fcb = {
            .userData_ = info,
            .callback_ = VSyncCallback,
        };
	auto refRecevier = weakReceiver.lock();
	if (refRecevier) {
          refRecevier->RequestNextVSync(fcb);
	}
    });
    
#else
    auto callback = [info](int64_t nanoTimestamp) {
        VSyncCallback(nanoTimestamp, (void*)info);
    };

    if (!vsync_scheduler_) {
        vsync_scheduler_ = OHOS::AGP::VsyncScheduler::Create();
    }

    auto vsync_scheduler = vsync_scheduler_;
    task_runners_.GetPlatformTaskRunner()->PostTask([vsync_scheduler, callback]() {
        if (vsync_scheduler) {
            vsync_scheduler->RequestVsync(callback);
        }
    });
#endif
}

float VsyncWaiterEmbedder::GetDisplayRefreshRate() const
{
#ifndef OHOS_STANDARD_SYSTEM
    JNIEnv* env = fml::jni::AttachCurrentThread();
    auto displayManagerClass = AceDisplayManagerJni::GetClass();
    if (displayManagerClass == nullptr) {
        return kUnknownRefreshRateFPS;
    }
    jclass clazz = displayManagerClass->obj();
    if (clazz == nullptr) {
        return kUnknownRefreshRateFPS;
    }
    jfieldID fid = env->GetStaticFieldID(clazz, "refreshRateFPS", "F");
    return env->GetStaticFloatField(clazz, fid);
#else
    return 60.0f;
#endif
}

} // namespace flutter
