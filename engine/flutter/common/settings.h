// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_COMMON_SETTINGS_H_
#define FLUTTER_COMMON_SETTINGS_H_

#include <fcntl.h>
#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

#include "flutter/fml/closure.h"
#include "flutter/fml/mapping.h"
#include "flutter/fml/time/time_point.h"
#include "flutter/fml/unique_fd.h"

namespace flutter {

enum class AcePlatform : int32_t {
  ACE_PLATFORM_INVALID = -1,
  ACE_PLATFORM_ANDROID,
  ACE_PLATFORM_IOS,
  ACE_PLATFORM_OHOS,
};

class FrameTiming {
 public:
  enum Phase { kBuildStart, kBuildFinish, kRasterStart, kRasterFinish, kCount };

  static constexpr Phase kPhases[kCount] = {kBuildStart, kBuildFinish,
                                            kRasterStart, kRasterFinish};

  fml::TimePoint Get(Phase phase) const { return data_[phase]; }
  fml::TimePoint Set(Phase phase, fml::TimePoint value) {
    return data_[phase] = value;
  }

 private:
  fml::TimePoint data_[kCount];
};

using TaskObserverAdd =
    std::function<void(intptr_t /* key */, fml::closure /* callback */)>;
using TaskObserverRemove = std::function<void(intptr_t /* key */)>;
using UnhandledExceptionCallback =
    std::function<bool(const std::string& /* error */,
                       const std::string& /* stack trace */)>;

// TODO(chinmaygarde): Deprecate all the "path" struct members in favor of the
// callback that generates the mapping from these paths.
// https://github.com/flutter/flutter/issues/26783
using MappingCallback = std::function<std::unique_ptr<fml::Mapping>(void)>;
using MappingsCallback =
    std::function<std::vector<std::unique_ptr<const fml::Mapping>>(void)>;

using FrameRasterizedCallback = std::function<void(const FrameTiming&)>;

struct Settings {
  Settings();

  Settings(const Settings& other);

  ~Settings();

  // Font settings
  bool use_test_fonts = false;

  // Engine settings
  TaskObserverAdd task_observer_add;
  TaskObserverRemove task_observer_remove;

  // The callback made on the UI thread in an isolate scope when the engine
  // detects that the framework is idle. The VM also uses this time to perform
  // tasks suitable when idling. Due to this, embedders are still advised to be
  // as fast as possible in returning from this callback. Long running
  // operations in this callback do have the capability of introducing jank.
  std::function<void(int64_t)> idle_notification_callback;
  // A callback given to the embedder to react to unhandled exceptions in the
  // running Flutter application. This callback is made on an internal engine
  // managed thread and embedders must re-thread as necessary. Performing
  // blocking calls in this callback will cause applications to jank.
  UnhandledExceptionCallback unhandled_exception_callback;
  bool enable_software_rendering = false;
  bool skia_deterministic_rendering_on_cpu = false;
  bool verbose_logging = false;
  std::string log_tag = "flutter";

  // The icu_initialization_required setting does not have a corresponding
  // switch because it is intended to be decided during build time, not runtime.
  // Some companies apply source modification here because their build system
  // brings its own ICU data files.
  bool icu_initialization_required = true;
  std::string icu_data_path;
  MappingCallback icu_mapper;

  // Assets settings
  fml::UniqueFD::element_type assets_dir =
      fml::UniqueFD::traits_type::InvalidValue();
  std::string assets_path;

  // Callback to handle the timings of a rasterized frame. This is called as
  // soon as a frame is rasterized.
  FrameRasterizedCallback frame_rasterized_callback;
  int32_t instanceId = 0;
  bool platform_as_ui_thread = false;
  bool use_current_event_runner = false;
  bool use_system_render_thread = false;
  bool use_io_thread = false;

  AcePlatform platform = AcePlatform::ACE_PLATFORM_INVALID;

  std::string ToString() const;
};

}  // namespace flutter

#endif  // FLUTTER_COMMON_SETTINGS_H_
