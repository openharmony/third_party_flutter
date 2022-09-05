// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Framework adapted to ACE.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#include "flutter/shell/common/vsync_waiter_fallback.h"

#include "flutter/fml/logging.h"

namespace flutter {
namespace {

static fml::TimePoint SnapToNextTick(fml::TimePoint value,
                                     fml::TimePoint tick_phase,
                                     fml::TimeDelta tick_interval) {
  fml::TimeDelta offset = (tick_phase - value) % tick_interval;
  if (offset != fml::TimeDelta::Zero())
    offset = offset + tick_interval;
  return value + offset;
}

}  // namespace

VsyncWaiterFallback::VsyncWaiterFallback(TaskRunners task_runners)
    : VsyncWaiter(std::move(task_runners)), phase_(fml::TimePoint::Now()) {}

VsyncWaiterFallback::~VsyncWaiterFallback() = default;

// |VsyncWaiter|
void VsyncWaiterFallback::AwaitVSync() {
// ACE PC preview
#if defined(PREVIEW)
  constexpr fml::TimeDelta kSingleFrameInterval =
      fml::TimeDelta::FromSecondsF(1.0 / 30.0);
#else
  constexpr fml::TimeDelta kSingleFrameInterval =
      fml::TimeDelta::FromSecondsF(1.0 / 60.0);
#endif

  auto next =
      SnapToNextTick(fml::TimePoint::Now(), phase_, kSingleFrameInterval);

  FireCallback(next, next + kSingleFrameInterval);
}

}  // namespace flutter
