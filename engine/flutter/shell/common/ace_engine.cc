// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Framework adapted to ACE.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "flutter/shell/common/engine.h"

#include "flutter/common/settings.h"
#include "flutter/fml/eintr_wrapper.h"
#include "flutter/fml/file.h"
#include "flutter/fml/make_copyable.h"
#include "flutter/fml/paths.h"
#include "flutter/fml/trace_event.h"
#include "flutter/fml/unique_fd.h"
#include "flutter/lib/snapshot/snapshot.h"
#include "flutter/lib/ui/text/font_collection.h"
#include "flutter/shell/common/animator.h"
#include "flutter/shell/common/platform_view.h"
#include "flutter/shell/common/shell.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkPictureRecorder.h"

namespace flutter {

static constexpr char kAssetChannel[] = "flutter/assets";
static constexpr char kLifecycleChannel[] = "flutter/lifecycle";
static constexpr char kNavigationChannel[] = "flutter/navigation";
static constexpr char kLocalizationChannel[] = "flutter/localization";
static constexpr char kSettingsChannel[] = "flutter/settings";

Engine::Engine(Delegate& delegate,
               TaskRunners task_runners,
               Settings settings,
               std::unique_ptr<Animator> animator,
               fml::WeakPtr<IOManager> io_manager)
    : delegate_(delegate),
      settings_(std::move(settings)),
      animator_(std::move(animator)),
      activity_running_(true),
      have_surface_(false),
      weak_factory_(this) {
  // Runtime controller is initialized here because it takes a reference to this
  // object as its delegate. The delegate may be called in the constructor and
  // we want to be fully initilazed by that point.
  runtime_controller_ = std::make_unique<RuntimeController>(
      *this,                                 // runtime delegate
      std::move(task_runners),               // task runners
      std::move(io_manager),                 // io manager
      settings.instanceId,                   // instance id
      settings_.idle_notification_callback   // idle notification callback
  );
}

Engine::~Engine() = default;

float Engine::GetDisplayRefreshRate() const {
  return animator_->GetDisplayRefreshRate();
}

fml::WeakPtr<Engine> Engine::GetWeakPtr() const {
  return weak_factory_.GetWeakPtr();
}

bool Engine::UpdateAssetManager(
    std::shared_ptr<AssetManager> new_asset_manager) {
  if (asset_manager_ == new_asset_manager) {
    return false;
  }

  asset_manager_ = new_asset_manager;

  if (!asset_manager_) {
    return false;
  }

  // Using libTXT as the text engine.
  auto& font_collection = GetFontCollection();
  font_collection.RegisterFonts(asset_manager_);

  if (settings_.use_test_fonts) {
    font_collection.RegisterTestFonts();
  }

  return true;
}

bool Engine::Restart(RunConfiguration configuration) {
  TRACE_EVENT0("flutter", "Engine::Restart");
  if (!configuration.IsValid()) {
    FML_LOG(ERROR) << "Engine run configuration was invalid.";
    return false;
  }
  delegate_.OnPreEngineRestart();
  runtime_controller_ = runtime_controller_->Clone();
  return Run(std::move(configuration)) == Engine::RunStatus::Success;
}

Engine::RunStatus Engine::Run(RunConfiguration configuration) {
  if (!configuration.IsValid()) {
    FML_LOG(ERROR) << "Engine run configuration was invalid.";
    return RunStatus::Failure;
  }
  return RunStatus::Success;
}

Engine::RunStatus Engine::PrepareAndLaunchIsolate(
    RunConfiguration configuration) {
  TRACE_EVENT0("flutter", "Engine::PrepareAndLaunchIsolate");

  return RunStatus::Success;
}

void Engine::BeginFrame(fml::TimePoint frame_time) {
  TRACE_EVENT0("flutter", "Engine::BeginFrame");
  runtime_controller_->BeginFrame(frame_time);
}

void Engine::ReportTimings(std::vector<int64_t> timings) {
  TRACE_EVENT0("flutter", "Engine::ReportTimings");
  runtime_controller_->ReportTimings(std::move(timings));
}

void Engine::NotifyIdle(int64_t deadline) {
  TRACE_EVENT1("flutter", "Engine::NotifyIdle", "deadline_now_delta",
               std::to_string(deadline).c_str());
  runtime_controller_->NotifyIdle(deadline);
}

void Engine::OnOutputSurfaceCreated() {
  have_surface_ = true;
  StartAnimatorIfPossible();
  ScheduleFrame();
}

void Engine::OnOutputSurfaceDestroyed() {
  have_surface_ = false;
  StopAnimator();
}

void Engine::SetIdleNotificationCallback(const IdleCallback& idleCallback) {
    runtime_controller_->SetIdleNotificationCallback(idleCallback);
}

void Engine::SetViewportMetrics(const ViewportMetrics& metrics) {
  bool dimensions_changed =
      viewport_metrics_.physical_height != metrics.physical_height ||
      viewport_metrics_.physical_width != metrics.physical_width ||
      viewport_metrics_.physical_depth != metrics.physical_depth;
  viewport_metrics_ = metrics;
  runtime_controller_->SetViewportMetrics(viewport_metrics_);
  if (animator_) {
    if (dimensions_changed)
      animator_->SetDimensionChangePending();
    if (have_surface_)
      ScheduleFrame();
  }
}

void Engine::DispatchPlatformMessage(fml::RefPtr<PlatformMessage> message) {
  if (message->channel() == kLifecycleChannel) {
    if (HandleLifecyclePlatformMessage(message.get()))
      return;
  } else if (message->channel() == kLocalizationChannel) {
    if (HandleLocalizationPlatformMessage(message.get()))
      return;
  } else if (message->channel() == kSettingsChannel) {
    HandleSettingsPlatformMessage(message.get());
    return;
  }

  // If there's no runtime_, we may still need to set the initial route.
  if (message->channel() == kNavigationChannel)
    HandleNavigationPlatformMessage(std::move(message));
}

bool Engine::HandleLifecyclePlatformMessage(PlatformMessage* message) {
  const auto& data = message->data();
  std::string state(reinterpret_cast<const char*>(data.data()), data.size());
  if (state == "AppLifecycleState.paused" ||
      state == "AppLifecycleState.suspending") {
    activity_running_ = false;
    StopAnimator();
  } else if (state == "AppLifecycleState.resumed" ||
             state == "AppLifecycleState.inactive") {
    activity_running_ = true;
    StartAnimatorIfPossible();
  }

  // Always schedule a frame when the app does become active as per API
  // recommendation
  // https://developer.apple.com/documentation/uikit/uiapplicationdelegate/1622956-applicationdidbecomeactive?language=objc
  if (state == "AppLifecycleState.resumed" && have_surface_) {
    ScheduleFrame();
  }
  runtime_controller_->SetLifecycleState(state);
  // Always forward these messages to the framework by returning false.
  return false;
}

bool Engine::HandleNavigationPlatformMessage(
    fml::RefPtr<PlatformMessage> message) {
  return true;
}

bool Engine::HandleLocalizationPlatformMessage(PlatformMessage* message) {
  return false;
}

void Engine::HandleSettingsPlatformMessage(PlatformMessage* message) {
  const auto& data = message->data();
  std::string jsonData(reinterpret_cast<const char*>(data.data()), data.size());
  if (runtime_controller_->SetUserSettingsData(std::move(jsonData)) &&
      have_surface_) {
    ScheduleFrame();
  }
}

void Engine::DispatchPointerDataPacket(const PointerDataPacket& packet,
                                       uint64_t trace_flow_id) {
  TRACE_EVENT0("flutter", "Engine::DispatchPointerDataPacket");
  TRACE_FLOW_STEP("flutter", "PointerEvent", trace_flow_id);
  animator_->EnqueueTraceFlowId(trace_flow_id);
  runtime_controller_->DispatchPointerDataPacket(packet);
}

void Engine::StopAnimator() {
  animator_->Stop();
}

void Engine::StartAnimatorIfPossible() {
  if (activity_running_ && have_surface_)
    animator_->Start();
}

std::string Engine::DefaultRouteName() {
  if (!initial_route_.empty()) {
    return initial_route_;
  }
  return "/";
}

void Engine::ScheduleFrame(bool regenerate_layer_tree) {
  animator_->RequestFrame(regenerate_layer_tree);
}

void Engine::Render(std::unique_ptr<flutter::LayerTree> layer_tree) {
  if (!layer_tree)
    return;

  SkISize frame_size = SkISize::Make(viewport_metrics_.physical_width,
                                     viewport_metrics_.physical_height);
  if (frame_size.isEmpty())
    return;

  layer_tree->set_frame_size(frame_size);
  animator_->Render(std::move(layer_tree));
}

void Engine::HandlePlatformMessage(fml::RefPtr<PlatformMessage> message) {
  if (message->channel() == kAssetChannel) {
    HandleAssetPlatformMessage(std::move(message));
  } else {
    delegate_.OnEngineHandlePlatformMessage(std::move(message));
  }
}

void Engine::SetNeedsReportTimings(bool needs_reporting) {
  delegate_.SetNeedsReportTimings(needs_reporting);
}

FontCollection& Engine::GetFontCollection() {
  std::call_once(font_flag_, [this]() {
    font_collection_ = std::make_unique<FontCollection>();
    if (font_collection_->GetFontCollection()) {
      std::string emptyLocale;
      // 0x4e2d is unicode for '中'.
      font_collection_->GetFontCollection()->MatchFallbackFont(0x4e2d, emptyLocale);
      font_collection_->GetFontCollection()->GetMinikinFontCollectionForFamilies({"sans-serif"}, emptyLocale);
    }
  });
  return *font_collection_;
}

void Engine::HandleAssetPlatformMessage(fml::RefPtr<PlatformMessage> message) {
}

}  // namespace flutter
