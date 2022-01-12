// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.10.08 platform task runner adapt ohos.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#include "flutter/shell/platform/ohos/platform_task_runner.h"

#include "flutter/shell/platform/ohos/platform_task_runner_adapter.h"

namespace flutter {

fml::RefPtr<fml::TaskRunner> PlatformTaskRunner::CurrentTaskRunner(bool useCurrentEventRunner)
{
    return PlatformTaskRunnerAdapter::CurrentTaskRunner(useCurrentEventRunner);
}

}  // namespace flutter