// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.4.30 platform task runner adapt ohos.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#ifndef FLUTTER_SHELL_PLATFORM_OHOS_PLATFORM_TASK_RUNNER_ADAPTER_H
#define FLUTTER_SHELL_PLATFORM_OHOS_PLATFORM_TASK_RUNNER_ADAPTER_H

#include "event_handler.h"
#include "event_runner.h"

#include "flutter/fml/message_loop_impl.h"
#include "flutter/fml/message_loop_task_queues.h"

namespace flutter {

class PlatformTaskRunnerAdapter : public fml::TaskRunner {
public:
    explicit PlatformTaskRunnerAdapter(bool useCurrentEventRunner);

    void PostTask(fml::closure task) override;

    void PostTaskForTime(fml::closure task, fml::TimePoint target_time) override;

    void PostDelayedTask(fml::closure task, fml::TimeDelta delay) override;

    bool RunsTasksOnCurrentThread() override;

    fml::TaskQueueId GetTaskQueueId() override;

    static fml::RefPtr<fml::TaskRunner> CurrentTaskRunner(bool useCurrentEventRunner = false);

private:
    static fml::RefPtr<fml::TaskRunner> taskRunner_;

    std::shared_ptr<OHOS::AppExecFwk::EventHandler> eventHandler_;
    std::shared_ptr<OHOS::AppExecFwk::EventRunner> eventRunner_;
};

}  // namespace flutter

#endif // FLUTTER_SHELL_PLATFORM_OHOS_PLATFORM_TASK_RUNNER_ADAPTER_H