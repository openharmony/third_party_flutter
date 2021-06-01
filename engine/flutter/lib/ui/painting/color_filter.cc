// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/color_filter.h"

namespace flutter {

IMPLEMENT_WRAPPERTYPEINFO(ui, ColorFilter);

#define FOR_EACH_BINDING(V)             \
  V(ColorFilter, initMode)              \
  V(ColorFilter, initMatrix)            \
  V(ColorFilter, initSrgbToLinearGamma) \
  V(ColorFilter, initLinearToSrgbGamma)

FOR_EACH_BINDING(DART_NATIVE_CALLBACK)

void ColorFilter::RegisterNatives(tonic::DartLibraryNatives* natives) {
}

fml::RefPtr<ColorFilter> ColorFilter::Create() {
  return fml::MakeRefCounted<ColorFilter>();
}

void ColorFilter::initMode(int color, int blend_mode) {
#ifdef USE_SYSTEM_SKIA
  filter_ = SkColorFilter::MakeModeFilter(static_cast<SkColor>(color),
                                  static_cast<SkBlendMode>(blend_mode));
#else
  filter_ = SkColorFilters::Blend(static_cast<SkColor>(color),
                                  static_cast<SkBlendMode>(blend_mode));
#endif
}

sk_sp<SkColorFilter> ColorFilter::MakeColorMatrixFilter255(
    const float array[20]) {
  float tmp[20];
  memcpy(tmp, array, sizeof(tmp));
  tmp[4] *= 1.0f / 255;
  tmp[9] *= 1.0f / 255;
  tmp[14] *= 1.0f / 255;
  tmp[19] *= 1.0f / 255;
#ifdef USE_SYSTEM_SKIA
  return SkColorFilter::MakeMatrixFilterRowMajor255(tmp);
#else
  return SkColorFilters::Matrix(tmp);
#endif
}

void ColorFilter::initMatrix(const tonic::Float32List& color_matrix) {
  FML_CHECK(color_matrix.size() == 20);

  filter_ = MakeColorMatrixFilter255(color_matrix.data());
}

void ColorFilter::initLinearToSrgbGamma() {
#ifdef USE_SYSTEM_SKIA
  filter_ = SkColorFilter::MakeLinearToSRGBGamma();
#else
  filter_ = SkColorFilters::LinearToSRGBGamma();
#endif
}

void ColorFilter::initSrgbToLinearGamma() {
#ifdef USE_SYSTEM_SKIA
  filter_ = SkColorFilter::MakeSRGBToLinearGamma();
#else
  filter_ = SkColorFilters::SRGBToLinearGamma();
#endif
}

ColorFilter::~ColorFilter() = default;

}  // namespace flutter
