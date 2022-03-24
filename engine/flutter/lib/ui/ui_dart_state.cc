// Copyright 2013-2022 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/ui_dart_state.h"

#include "flutter/runtime/window_manager.h"

namespace flutter {

constexpr int32_t MIN_PLUGIN_SUBCONTAINER_ID = 2000000;
static std::unique_ptr<UIDartState> g_ui_state;

UIDartState::UIDartState() {
}

UIDartState::~UIDartState() {
}

UIDartState* UIDartState::Current() {
  if (!g_ui_state) {
    FML_LOG(ERROR) << "UIDartState not initialize!";
    return nullptr;
  }
  return g_ui_state.get();
}

void UIDartState::Init(int32_t instanceId,
                       fml::WeakPtr<IOManager> io_manager,
                       TaskRunners task_runners) {
  if (!g_ui_state) {
    g_ui_state.reset(new UIDartState());
  }
  auto item = std::make_unique<UIDartState::UIStateItem>(std::move(io_manager), std::move(task_runners));
  g_ui_state->SetCurInstance(instanceId);
  g_ui_state->SetCurStateItem(instanceId, std::move(item));
}

void UIDartState::DeInit(int32_t instanceId) {
  g_ui_state->RemoveStateItem(instanceId);
  g_ui_state->RemovePluginParentContainer(instanceId);
}

void UIDartState::SetCurStateItem(int32_t id, std::unique_ptr<UIDartState::UIStateItem> item) {
  FML_LOG(INFO) << "SetCurStateItem id:" << id;
  std::lock_guard<std::mutex> lock(mutex_);
  auto iter = state_map_.find(id);
  if (iter == state_map_.end()) {
    state_map_.emplace(id, std::move(item));
  } else {
    FML_LOG(WARNING) << "already have item of this instance id:" << id;
  }
}

void UIDartState::RemoveStateItem(int32_t id) {
  std::lock_guard<std::mutex> lock(mutex_);
  state_map_.erase(id);
}

UIDartState::UIStateItem* UIDartState::GetStateById(int32_t id) const {
  if (id >= MIN_PLUGIN_SUBCONTAINER_ID) {
      id = GetPluginParentContainerId(id);
  }
  std::lock_guard<std::mutex> lock(mutex_);
  auto iter = state_map_.find(id);
  if (iter != state_map_.end()) {
    return iter->second.get();
  } else {
    return nullptr;
  }
}

fml::RefPtr<SkiaUnrefQueue> UIDartState::GetSkiaUnrefQueue() const {
  int32_t *ptr =  cur_instance_id_.get();
  if (ptr == nullptr) {
    FML_LOG(FATAL) << "UIDartState cur id is null";
  }
  int32_t id = *ptr;
  auto item = GetStateById(id);
  if (item == nullptr) {
    FML_LOG(FATAL) << "UIDartState cur item is null";
  }
  return item->GetSkiaUnrefQueue();
}

fml::WeakPtr<IOManager> UIDartState::GetIOManager() const {
  int32_t *ptr =  cur_instance_id_.get();
  if (ptr == nullptr) {
    FML_LOG(FATAL) << "UIDartState cur id is null";
  }
  int32_t id = *ptr;
  auto item = GetStateById(id);
  if (item == nullptr) {
    FML_LOG(FATAL) << "UIDartState cur item is null";
  }
  return item->GetIOManager();
}

const TaskRunners& UIDartState::GetTaskRunners() const {
  int32_t *ptr =  cur_instance_id_.get();
  if (ptr == nullptr) {
    FML_LOG(FATAL) << "UIDartState cur id is null";
  }
  int32_t id = *ptr;
  auto item = GetStateById(id);
  if (item == nullptr) {
    FML_LOG(FATAL) << "UIDartState cur item is null";
  }
  return item->GetTaskRunners();
}

Window* UIDartState::window() const {
  int32_t *ptr =  cur_instance_id_.get();
  if (ptr == nullptr) {
    FML_LOG(ERROR) << "UIDartState cur id is null";
    return nullptr;
  }
  int32_t id = *ptr;
  if (id >= MIN_PLUGIN_SUBCONTAINER_ID) {
    id = GetPluginParentContainerId(id);
  }
  return WindowManager::GetWindow(id);
}

int64_t UIDartState::GetPluginParentContainerId(int64_t pluginId) const
{
    std::lock_guard<std::mutex> lock(parentContainerMutex_);
    auto result = parentContainerMap_.find(pluginId);
    if (result != parentContainerMap_.end()) {
      return result->second;
    } else {
      FML_LOG(ERROR) << "ParentContainerId is empty.";
      return 0;
    }
}

void UIDartState::AddPluginParentContainer(int64_t pluginId, int32_t pluginParentContainerId)
{
    std::lock_guard<std::mutex> lock(parentContainerMutex_);
    auto result = parentContainerMap_.try_emplace(pluginId, pluginParentContainerId);
    if (!result.second) {
      FML_LOG(ERROR) << "already have pluginSubContainer of this instance, pluginId: " << pluginId;
    }
}

void UIDartState::RemovePluginParentContainer(int64_t pluginParentContainerId)
{
    std::lock_guard<std::mutex> lock(parentContainerMutex_);
    auto iter = parentContainerMap_.begin();
    while(iter != parentContainerMap_.end()) {
      if (iter->second == pluginParentContainerId) {
        parentContainerMap_.erase(iter++);
      } else {
        ++iter;
      }
    }
}

} // namespcae blink

