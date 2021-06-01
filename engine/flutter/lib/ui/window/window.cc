// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/window/window.h"

#include "flutter/lib/ui/compositing/scene.h"

namespace flutter {

Window::Window(WindowClient* client) : client_(client) {}

void Window::DidCreateIsolate() {}

void Window::UpdateWindowMetrics(const ViewportMetrics& metrics) {
  viewport_metrics_ = metrics;
}

void Window::UpdateUserSettingsData(const std::string& data) {
}

void Window::DispatchPointerDataPacket(const PointerDataPacket& packet) {
}

void Window::BeginFrame(fml::TimePoint frameTime) {
  if (begin_frame_callback_) {
      begin_frame_callback_(frameTime.ToEpochDelta().ToNanoseconds());
  }
}

void Window::ReportTimings(std::vector<int64_t> timings) {
}

void Window::ScheduleFrame() {
  client()->ScheduleFrame();
}

}  // namespace flutter
