// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Framework adapted to ACE.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#include <memory>

#include "flutter/shell/common/vsync_waiter.h"

#ifndef SHELL_COMMON_VSYNC_CREATOR_H_
#define SHELL_COMMON_VSYNC_CREATOR_H_
namespace flutter {

class VsyncCreator {
public:
    static std::unique_ptr<VsyncWaiter> Create(flutter::TaskRunners);
};

} // namespace flutter
#endif // SHELL_COMMON_VSYNC_CREATOR_H_
