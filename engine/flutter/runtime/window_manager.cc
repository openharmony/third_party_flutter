// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Framework adapted to ACE.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#include "flutter/runtime/window_manager.h"

#include "flutter/fml/logging.h"

namespace flutter {

std::map<int32_t, std::unique_ptr<Window>> WindowManager::window_map_;
std::mutex WindowManager::mutex_;

void WindowManager::AddWindow(int32_t instance_id, std::unique_ptr<Window> window) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto iter = window_map_.find(instance_id);
  if (iter == window_map_.end()) {
    window_map_.emplace(instance_id, std::move(window));
  } else {
    FML_LOG(WARNING) << "already have window of this instance id:" << instance_id;
  }
}

Window* WindowManager::GetWindow(int32_t instance_id) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto iter = window_map_.find(instance_id);
  if (iter != window_map_.end()) {
    return iter->second.get();
  } else {
    return nullptr;
  }
}

void WindowManager::RemoveWindow(int32_t instance_id) {
  std::lock_guard<std::mutex> lock(mutex_);
  window_map_.erase(instance_id);
}

} // namespace flutter
