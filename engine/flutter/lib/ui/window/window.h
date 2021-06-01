// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_WINDOW_WINDOW_H_
#define FLUTTER_LIB_UI_WINDOW_WINDOW_H_

#include <string>
#include <unordered_map>
#include <vector>

#include "flutter/fml/time/time_point.h"
#include "flutter/lib/ui/window/pointer_data_packet.h"
#include "flutter/lib/ui/window/viewport_metrics.h"
#include "flutter/lib/ui/window/platform_message.h"

namespace flutter {
class FontCollection;
class Scene;

class WindowClient {
 public:
  virtual std::string DefaultRouteName() = 0;
  virtual void ScheduleFrame() = 0;
  virtual void Render(Scene* scene) = 0;
  virtual void HandlePlatformMessage(fml::RefPtr<PlatformMessage> message) = 0;
  virtual FontCollection& GetFontCollection() = 0;
  virtual void SetNeedsReportTimings(bool value) = 0;

 protected:
  virtual ~WindowClient() = default;
};

class Window final {
 public:
  explicit Window(WindowClient* client);

  ~Window() = default;

  WindowClient* client() const { return client_; }

  const ViewportMetrics& viewport_metrics() { return viewport_metrics_; }

  void DidCreateIsolate();
  void UpdateWindowMetrics(const ViewportMetrics& metrics);
  void UpdateUserSettingsData(const std::string& data);
  void DispatchPointerDataPacket(const PointerDataPacket& packet);
  void BeginFrame(fml::TimePoint frameTime);
  void ReportTimings(std::vector<int64_t> timings);

  // adapters to platform window
  using BeginFrameCallback = std::function<void(uint64_t)>;
  void ScheduleFrame();
  bool HasBeginFrameCallback() { return begin_frame_callback_ != nullptr; }
  void SetBeginFrameCallback(const BeginFrameCallback& callback) {
      begin_frame_callback_ = callback;
  }

 private:
  WindowClient* client_;
  ViewportMetrics viewport_metrics_;
  BeginFrameCallback begin_frame_callback_;
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_WINDOW_WINDOW_H_
