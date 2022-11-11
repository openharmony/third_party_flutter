// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.4.30 platform view adapt ohos.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#define FML_USED_ON_EMBEDDER

#include "flutter/shell/platform/ohos/ohos_shell_holder.h"

#include <pthread.h>
#include <sys/resource.h>
#include <sys/time.h>

#include <sstream>
#include <string>
#include <utility>

#include "flutter/fml/make_copyable.h"
#include "flutter/fml/message_loop.h"
#include "flutter/shell/common/rasterizer.h"
#include "flutter/shell/platform/ohos/platform_task_runner_adapter.h"
#include "flutter/shell/platform/ohos/platform_view_ohos.h"

namespace flutter {

OhosShellHolder::OhosShellHolder(
    flutter::Settings settings,
    bool is_background_view)
    : settings_(std::move(settings))
{
  // settings_.
  static size_t shell_count = 1;
  auto thread_label = std::to_string(shell_count++);

  uint64_t type_mask = 0;
  if (is_background_view) {
    type_mask |= ThreadHost::Type::UI;
  } else {
    if (!settings_.platform_as_ui_thread) {
      type_mask |= ThreadHost::Type::UI;
    }
    if (!settings_.use_system_render_thread) {
      type_mask |= ThreadHost::Type::GPU;
    }
    if (settings_.use_io_thread) {
      type_mask |= ThreadHost::Type::IO;
    }
    thread_host_ = {thread_label, type_mask};
  }

  fml::WeakPtr<PlatformViewOhos> weak_platform_view;
  Shell::CreateCallback<PlatformView> on_create_platform_view =
      [&weak_platform_view](Shell& shell) {
        std::unique_ptr<PlatformViewOhos> platform_view_ohos;
        if (shell.GetSettings().enable_software_rendering) {
          platform_view_ohos = std::make_unique<PlatformViewOhos>(shell, shell.GetTaskRunners(), true);
        }
        weak_platform_view = platform_view_ohos->GetWeakPtr();
        return platform_view_ohos;
      };

  Shell::CreateCallback<Rasterizer> on_create_rasterizer = [](Shell& shell) {
    return std::make_unique<Rasterizer>(shell, shell.GetTaskRunners());
  };

  // The current thread will be used as the platform thread. Ensure that the
  // message loop is initialized.
  fml::MessageLoop::EnsureInitializedForCurrentThread();
  fml::RefPtr<fml::TaskRunner> gpu_runner;
  fml::RefPtr<fml::TaskRunner> ui_runner;
  fml::RefPtr<fml::TaskRunner> io_runner;
  fml::RefPtr<fml::TaskRunner> platform_runner =
    PlatformTaskRunnerAdapter::CurrentTaskRunner(settings_.use_current_event_runner);
  if (is_background_view) {
    auto single_task_runner = thread_host_.ui_thread->GetTaskRunner();
    gpu_runner = single_task_runner;
    ui_runner = single_task_runner;
    io_runner = single_task_runner;
  } else {
    if (settings_.platform_as_ui_thread) {
      ui_runner = platform_runner;
    } else {
      ui_runner = thread_host_.ui_thread->GetTaskRunner();
    }
    if (!settings_.use_system_render_thread) {
      gpu_runner = thread_host_.gpu_thread->GetTaskRunner();
    } else {
      gpu_runner = ui_runner;
    }
    if (settings_.use_io_thread) {
      io_runner = thread_host_.io_thread->GetTaskRunner();
    } else {
      io_runner = ui_runner;
    }
  }
  flutter::TaskRunners task_runners(thread_label,     // label
                                    platform_runner,  // platform
                                    gpu_runner,       // gpu
                                    ui_runner,        // ui
                                    io_runner         // io
  );

  shell_ =
      Shell::Create(task_runners,             // task runners
                    settings_,                // settings
                    on_create_platform_view,  // platform view create callback
                    on_create_rasterizer      // rasterizer create callback
      );

  platform_view_ = weak_platform_view;
  FML_DCHECK(platform_view_);

  is_valid_ = shell_ != nullptr;

  if (is_valid_) {
    task_runners.GetGPUTaskRunner()->PostTask([]() {
      if (::setpriority(PRIO_PROCESS, gettid(), -5) != 0) {
        if (::setpriority(PRIO_PROCESS, gettid(), -2) != 0) {
          FML_LOG(ERROR) << "Failed to set GPU task runner priority";
        }
      }
    });
    task_runners.GetUITaskRunner()->PostTask([]() {
      if (::setpriority(PRIO_PROCESS, gettid(), -1) != 0) {
        FML_LOG(ERROR) << "Failed to set UI task runner priority";
      }
    });
  }
}

OhosShellHolder::~OhosShellHolder()
{
  shell_.reset();
  thread_host_.Reset();
}

bool OhosShellHolder::IsValid() const
{
  return is_valid_;
}

const flutter::Settings& OhosShellHolder::GetSettings() const
{
  return settings_;
}

void OhosShellHolder::Launch(RunConfiguration config)
{
  if (!IsValid()) {
    return;
  }

  shell_->RunEngine(std::move(config));
}

Rasterizer::Screenshot OhosShellHolder::Screenshot(
    Rasterizer::ScreenshotType type,
    bool base64_encode)
{
  if (!IsValid()) {
    return {nullptr, SkISize::MakeEmpty()};
  }
  return shell_->Screenshot(type, base64_encode);
}

fml::WeakPtr<PlatformViewOhos> OhosShellHolder::GetPlatformView()
{
  FML_DCHECK(platform_view_);
  return platform_view_;
}

}  // namespace flutter
