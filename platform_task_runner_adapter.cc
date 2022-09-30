// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.4.30 platform task runner adapt ohos.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#include "flutter/shell/platform/ohos/platform_task_runner_adapter.h"

namespace flutter {

fml::RefPtr<fml::TaskRunner> PlatformTaskRunnerAdapter::taskRunner_;

PlatformTaskRunnerAdapter::PlatformTaskRunnerAdapter(bool useCurrentEventRunner)
    : fml::TaskRunner(nullptr)
{
    if (useCurrentEventRunner) {
        eventRunner_ = OHOS::AppExecFwk::EventRunner::Current();
    } else {
        eventRunner_ = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
    }
    eventHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(eventRunner_);
}

void PlatformTaskRunnerAdapter::PostTask(fml::closure task)
{
    eventHandler_->PostTask(std::move(task));
}

void PlatformTaskRunnerAdapter::PostTaskForTime(fml::closure task, fml::TimePoint target_time)
{
    eventHandler_->PostTimingTask(std::move(task), target_time.ToEpochDelta().ToMilliseconds(), "");
}

void PlatformTaskRunnerAdapter::PostDelayedTask(fml::closure task, fml::TimeDelta delay)
{
    eventHandler_->PostTask(std::move(task), delay.ToMilliseconds());
}

bool PlatformTaskRunnerAdapter::RunsTasksOnCurrentThread()
{
    return eventRunner_->IsCurrentRunnerThread();
}

fml::TaskQueueId PlatformTaskRunnerAdapter::GetTaskQueueId()
{
    return fml::_kUnmerged;
}

fml::RefPtr<fml::TaskRunner> PlatformTaskRunnerAdapter::CurrentTaskRunner(bool useCurrentEventRunner)
{
    if (useCurrentEventRunner) {
        return fml::MakeRefCounted<PlatformTaskRunnerAdapter>(useCurrentEventRunner);
    }
    if (taskRunner_) {
        return taskRunner_;
    }
    taskRunner_ = fml::MakeRefCounted<PlatformTaskRunnerAdapter>(useCurrentEventRunner);
    return taskRunner_;
}

}  // namespace flutter