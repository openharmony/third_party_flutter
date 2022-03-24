// Copyright 2013-2022 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_UI_DART_STATE_H_
#define FLUTTER_LIB_UI_UI_DART_STATE_H_

#include "third_party/skia/include/gpu/GrContext.h"

#include <map>
#include <memory>

#include "flutter/common/task_runners.h"
#include "flutter/flow/skia_gpu_object.h"
#include "flutter/fml/memory/weak_ptr.h"
#include "flutter/fml/thread_local.h"
#include "flutter/lib/ui/io_manager.h"
#include "flutter/lib/ui/window/window.h"

namespace flutter {

class Window;

class __attribute__((visibility("default"))) UIDartState {
 public:
  ~UIDartState();
  static UIDartState* Current();

  class UIStateItem {
   public:
    UIStateItem(fml::WeakPtr<IOManager> io_manager, TaskRunners task_runners) : io_manager_(std::move(io_manager)),
        task_runners_(std::move(task_runners)) {}
    const TaskRunners& GetTaskRunners() const { return task_runners_; }
    fml::RefPtr<SkiaUnrefQueue> GetSkiaUnrefQueue() const {
      return io_manager_->GetSkiaUnrefQueue();
    }
    fml::WeakPtr<IOManager> GetIOManager() const {
      return io_manager_;
    }
   private:
    fml::WeakPtr<IOManager> io_manager_;
    const TaskRunners task_runners_;
  };

  static void Init(int32_t instanceId,
            fml::WeakPtr<IOManager> io_manager,
            TaskRunners task_runners);
  static void DeInit(int32_t instanceId);

  fml::RefPtr<SkiaUnrefQueue> GetSkiaUnrefQueue() const;
  fml::WeakPtr<IOManager> GetIOManager() const;
  Window* window() const;
  const TaskRunners& GetTaskRunners() const;
  UIStateItem* GetStateById(int32_t id) const;

  template <class T>
  static SkiaGPUObject<T> CreateGPUObject(sk_sp<T> object) {
    if (!object) {
      return {};
    }

    auto state = UIDartState::Current();
    auto queue = state->GetSkiaUnrefQueue();
    return {std::move(object), std::move(queue)};
  };

  void SetCurInstance(int32_t id) {
    int32_t* ptr = new int32_t(id);
    cur_instance_id_.reset(ptr);
  }

  void SetCurStateItem(int32_t id, std::unique_ptr<UIStateItem>);
  void RemoveStateItem(int32_t id);

  void AddPluginParentContainer(int64_t pluginId, int32_t pluginParentContainerId);
  void RemovePluginParentContainer(int64_t pluginParentContainerId);
  int64_t GetPluginParentContainerId(int64_t pluginId) const;

 private:
  UIDartState();
  fml::ThreadLocalUniquePtr<int32_t> cur_instance_id_;
  std::map<int32_t, std::unique_ptr<UIStateItem>> state_map_;
  mutable std::mutex mutex_;
  std::unordered_map<int64_t, int32_t> parentContainerMap_;
  mutable std::mutex parentContainerMutex_;
  FML_DISALLOW_COPY_AND_ASSIGN(UIDartState);
};

} // namespace blink

#endif // FLUTTER_LIB_UI_UI_DART_STATE_H_
