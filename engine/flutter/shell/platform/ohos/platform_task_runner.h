// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.10.08 platform task runner adapt ohos.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#ifndef FLUTTER_SHELL_PLATFORM_OHOS_PLATFORM_TASK_RUNNER_H
#define FLUTTER_SHELL_PLATFORM_OHOS_PLATFORM_TASK_RUNNER_H

#include "flutter/fml/message_loop_impl.h"
#include "flutter/fml/message_loop_task_queues.h"

namespace flutter {

class PlatformTaskRunner {
public:
    static fml::RefPtr<fml::TaskRunner> CurrentTaskRunner(bool useCurrentEventRunner = false);
};

}  // namespace flutter

#endif // FLUTTER_SHELL_PLATFORM_OHOS_PLATFORM_TASK_RUNNER_H