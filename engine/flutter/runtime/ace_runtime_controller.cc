// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Framework adapted to ACE.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#include "flutter/runtime/runtime_controller.h"

#include "flutter/fml/message_loop.h"
#include "flutter/fml/trace_event.h"
#include "flutter/lib/ui/compositing/scene.h"
#include "flutter/lib/ui/ui_dart_state.h"
#include "flutter/lib/ui/window/window.h"
#include "flutter/runtime/runtime_delegate.h"
#include "flutter/runtime/window_manager.h"

namespace flutter {

RuntimeController::RuntimeController(
    RuntimeDelegate& p_client,
    TaskRunners p_task_runners,
    fml::WeakPtr<IOManager> p_io_manager,
    int32_t instance_id,
    std::function<void(int64_t)> p_idle_notification_callback)
    : RuntimeController(p_client,
                        std::move(p_task_runners),
                        std::move(p_io_manager),
                        instance_id,
                        p_idle_notification_callback,
                        WindowData{/* default window data */}) {}

RuntimeController::RuntimeController(
    RuntimeDelegate& p_client,
    TaskRunners p_task_runners,
    fml::WeakPtr<IOManager> p_io_manager,
    int32_t instance_id,
    std::function<void(int64_t)> idle_notification_callback,
    WindowData p_window_data)
    : client_(p_client),
      task_runners_(p_task_runners),
      io_manager_(p_io_manager),
      instance_id_(instance_id),
      idle_notification_callback_(idle_notification_callback),
      window_data_(std::move(p_window_data)) {
    std::unique_ptr<Window> window = std::make_unique<Window>(this);
    WindowManager::AddWindow(instance_id, std::move(window));
    UIDartState::Init(instance_id, io_manager_, p_task_runners);
}

RuntimeController::~RuntimeController() {
  WindowManager::RemoveWindow(instance_id_);
  UIDartState::DeInit(instance_id_);
}

std::unique_ptr<RuntimeController> RuntimeController::Clone() const {
  return std::unique_ptr<RuntimeController>(new RuntimeController(
      client_,                      //
      task_runners_,                //
      io_manager_,                  //
      instance_id_,                 //
      idle_notification_callback_,  //
      window_data_));
}

bool RuntimeController::FlushRuntimeStateToIsolate() {
  return SetViewportMetrics(window_data_.viewport_metrics) &&
         SetLocales(window_data_.locale_data) &&
         SetSemanticsEnabled(window_data_.semantics_enabled) &&
         SetAccessibilityFeatures(window_data_.accessibility_feature_flags_) &&
         SetUserSettingsData(window_data_.user_settings_data) &&
         SetLifecycleState(window_data_.lifecycle_state);
}

void RuntimeController::SetIdleNotificationCallback(
    const IdleCallback& idle_notification_callback) {
  idle_notification_callback_ = idle_notification_callback;
}

bool RuntimeController::SetViewportMetrics(const ViewportMetrics& metrics) {
  window_data_.viewport_metrics = metrics;

  if (auto* window = GetWindowIfAvailable()) {
    window->UpdateWindowMetrics(metrics);
    return true;
  }
  return false;
}

bool RuntimeController::SetLocales(
    const std::vector<std::string>& locale_data) {
  return false;
}

bool RuntimeController::SetUserSettingsData(const std::string& data) {
  window_data_.user_settings_data = data;
  return false;
}

bool RuntimeController::SetLifecycleState(const std::string& data) {
  window_data_.lifecycle_state = data;

  return false;
}

bool RuntimeController::BeginFrame(fml::TimePoint frame_time) {
  if (auto* window = GetWindowIfAvailable()) {
    window->BeginFrame(frame_time);
    return true;
  }
  return false;
}

bool RuntimeController::ReportTimings(std::vector<int64_t> timings) {
  if (auto* window = GetWindowIfAvailable()) {
    window->ReportTimings(std::move(timings));
    return true;
  }
  return false;
}

bool RuntimeController::NotifyIdle(int64_t deadline) {
  // Idle notifications being in isolate scope are part of the contract.
  if (idle_notification_callback_) {
    TRACE_EVENT0("flutter", "EmbedderIdleNotification");
    idle_notification_callback_(deadline);
  }
  return true;
}

bool RuntimeController::DispatchPointerDataPacket(
    const PointerDataPacket& packet) {
  if (auto* window = GetWindowIfAvailable()) {
    TRACE_EVENT1("flutter", "RuntimeController::DispatchPointerDataPacket",
                 "mode", "basic");
    window->DispatchPointerDataPacket(packet);
    return true;
  }
  return false;
}

Window* RuntimeController::GetWindowIfAvailable() {
  return WindowManager::GetWindow(instance_id_);
}

std::string RuntimeController::DefaultRouteName() {
  return client_.DefaultRouteName();
}

void RuntimeController::ScheduleFrame() {
  client_.ScheduleFrame();
}

void RuntimeController::Render(Scene* scene) {
  client_.Render(scene->takeLayerTree());
}

void RuntimeController::HandlePlatformMessage(
    fml::RefPtr<PlatformMessage> message) {
  client_.HandlePlatformMessage(std::move(message));
}

FontCollection& RuntimeController::GetFontCollection() {
  return client_.GetFontCollection();
}

void RuntimeController::SetNeedsReportTimings(bool value) {
  client_.SetNeedsReportTimings(value);
}

RuntimeController::Locale::Locale(std::string language_code_,
                                  std::string country_code_,
                                  std::string script_code_,
                                  std::string variant_code_)
    : language_code(language_code_),
      country_code(country_code_),
      script_code(script_code_),
      variant_code(variant_code_) {}

RuntimeController::Locale::~Locale() = default;

RuntimeController::WindowData::WindowData() = default;

RuntimeController::WindowData::WindowData(const WindowData& other) = default;

RuntimeController::WindowData::~WindowData() = default;

}  // namespace flutter
