// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/common/settings.h"

#include <sstream>

namespace flutter {

constexpr FrameTiming::Phase FrameTiming::kPhases[FrameTiming::kCount];

Settings::Settings() = default;

Settings::Settings(const Settings& other) = default;

Settings::~Settings() = default;

std::string Settings::ToString() const {
  std::stringstream stream;
  stream << "Settings: " << std::endl;
  stream << "enable_software_rendering: " << enable_software_rendering
         << std::endl;
  stream << "log_tag: " << log_tag << std::endl;
  stream << "icu_initialization_required: " << icu_initialization_required
         << std::endl;
  stream << "icu_data_path: " << icu_data_path << std::endl;
  stream << "assets_dir: " << assets_dir << std::endl;
  stream << "assets_path: " << assets_path << std::endl;
  stream << "frame_rasterized_callback set: " << !!frame_rasterized_callback
         << std::endl;
  stream << "instance id:" << instanceId << std::endl;
  return stream.str();
}

}  // namespace flutter
