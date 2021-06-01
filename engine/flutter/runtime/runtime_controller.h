// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_RUNTIME_ACE_RUNTIME_CONTROLLER_H_
#define FLUTTER_RUNTIME_ACE_RUNTIME_CONTROLLER_H_

#include <memory>
#include <vector>

#include "flutter/common/task_runners.h"
#include "flutter/flow/layers/layer_tree.h"
#include "flutter/fml/macros.h"
#include "flutter/lib/ui/io_manager.h"
#include "flutter/lib/ui/text/font_collection.h"
#include "flutter/lib/ui/window/pointer_data_packet.h"
#include "flutter/lib/ui/window/window.h"

namespace flutter {
class Scene;
class RuntimeDelegate;
class View;
class Window;

class RuntimeController final : public WindowClient {
 public:
  using IdleCallback = std::function<void(int64_t)>;

  RuntimeController(RuntimeDelegate& client,
                    TaskRunners task_runners,
                    fml::WeakPtr<IOManager> io_manager,
                    int32_t instance_id,
                    std::function<void(int64_t)> idle_notification_callback);

  ~RuntimeController() override;

  std::unique_ptr<RuntimeController> Clone() const;

  void SetIdleNotificationCallback(const IdleCallback& idle_notification_callback);

  bool SetViewportMetrics(const ViewportMetrics& metrics);

  bool SetLocales(const std::vector<std::string>& locale_data);

  bool SetUserSettingsData(const std::string& data);

  bool SetLifecycleState(const std::string& data);

  bool SetSemanticsEnabled(bool enabled);

  bool SetAccessibilityFeatures(int32_t flags);

  bool BeginFrame(fml::TimePoint frame_time);

  bool ReportTimings(std::vector<int64_t> timings);

  bool NotifyIdle(int64_t deadline);

  bool IsRootIsolateRunning() const;

  bool DispatchPlatformMessage(fml::RefPtr<PlatformMessage> message);

  bool DispatchPointerDataPacket(const PointerDataPacket& packet);

 private:
  struct Locale {
    Locale(std::string language_code_,
           std::string country_code_,
           std::string script_code_,
           std::string variant_code_);

    ~Locale();

    std::string language_code;
    std::string country_code;
    std::string script_code;
    std::string variant_code;
  };

  // Stores data about the window to be used at startup
  // as well as on hot restarts. Data kept here will persist
  // after hot restart.
  struct WindowData {
    WindowData();

    WindowData(const WindowData& other);

    ~WindowData();

    ViewportMetrics viewport_metrics;
    std::string language_code;
    std::string country_code;
    std::string script_code;
    std::string variant_code;
    std::vector<std::string> locale_data;
    std::string user_settings_data = "{}";
    std::string lifecycle_state;
    bool semantics_enabled = false;
    bool assistive_technology_enabled = false;
    int32_t accessibility_feature_flags_ = 0;
  };

  RuntimeDelegate& client_;
  TaskRunners task_runners_;
  fml::WeakPtr<IOManager> io_manager_;
  std::string advisory_script_uri_;
  std::string advisory_script_entrypoint_;
  int32_t instance_id_;
  std::function<void(int64_t)> idle_notification_callback_;
  WindowData window_data_;

  RuntimeController(RuntimeDelegate& client,
                    TaskRunners task_runners,
                    fml::WeakPtr<IOManager> io_manager,
                    int32_t instance_id,
                    std::function<void(int64_t)> idle_notification_callback,
                    WindowData data);

  Window* GetWindowIfAvailable();

  bool FlushRuntimeStateToIsolate();

  // |WindowClient|
  std::string DefaultRouteName() override;

  // |WindowClient|
  void ScheduleFrame() override;

  // |WindowClient|
  void Render(Scene* scene) override;

  // |WindowClient|
  void HandlePlatformMessage(fml::RefPtr<PlatformMessage> message) override;

  // |WindowClient|
  FontCollection& GetFontCollection() override;

  // |WindowClient|
  void SetNeedsReportTimings(bool value) override;

  FML_DISALLOW_COPY_AND_ASSIGN(RuntimeController);
};

}  // namespace flutter

#endif  // FLUTTER_RUNTIME_RUNTIME_CONTROLLER_H_
