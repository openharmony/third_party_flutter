// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ACE PC preivew.
#ifndef WINDOWS_PLATFORM
// It cannot be passed to the sub-include of math.h, so define it in gn.
#define _USE_MATH_DEFINES
#endif

#include "flutter/lib/ui/painting/gradient.h"

namespace flutter {

typedef CanvasGradient
    Gradient;  // Because the C++ name doesn't match the Dart name.

IMPLEMENT_WRAPPERTYPEINFO(ui, Gradient);

#define FOR_EACH_BINDING(V) \
  V(Gradient, initLinear)   \
  V(Gradient, initRadial)   \
  V(Gradient, initSweep)    \
  V(Gradient, initTwoPointConical)

FOR_EACH_BINDING(DART_NATIVE_CALLBACK)

void CanvasGradient::RegisterNatives(tonic::DartLibraryNatives* natives) {
}

fml::RefPtr<CanvasGradient> CanvasGradient::Create() {
  return fml::MakeRefCounted<CanvasGradient>();
}

void CanvasGradient::initLinear(const tonic::Float32List& end_points,
                                const tonic::Int32List& colors,
                                const tonic::Float32List& color_stops,
                                SkTileMode tile_mode,
                                const tonic::Float64List& matrix4) {
  FML_DCHECK(end_points.size() == 4);
  FML_DCHECK(colors.size() == color_stops.size() ||
             color_stops.data() == nullptr);

  static_assert(sizeof(SkPoint) == sizeof(float) * 2,
                "SkPoint doesn't use floats.");
  static_assert(sizeof(SkColor) == sizeof(int32_t),
                "SkColor doesn't use int32_t.");

  SkMatrix sk_matrix;
  bool has_matrix = matrix4.data() != nullptr;
  if (has_matrix) {
    sk_matrix = ToSkMatrix(matrix4);
  }

  set_shader(UIDartState::CreateGPUObject(SkGradientShader::MakeLinear(
      reinterpret_cast<const SkPoint*>(end_points.data()),
      reinterpret_cast<const SkColor*>(colors.data()), color_stops.data(),
      colors.size(), tile_mode, 0, has_matrix ? &sk_matrix : nullptr)));
}

void CanvasGradient::initRadial(double center_x,
                                double center_y,
                                double radius,
                                const tonic::Int32List& colors,
                                const tonic::Float32List& color_stops,
                                SkTileMode tile_mode,
                                const tonic::Float64List& matrix4) {
  FML_DCHECK(colors.size() == color_stops.size() ||
             color_stops.data() == nullptr);

  static_assert(sizeof(SkColor) == sizeof(int32_t),
                "SkColor doesn't use int32_t.");

  SkMatrix sk_matrix;
  bool has_matrix = matrix4.data() != nullptr;
  if (has_matrix) {
    sk_matrix = ToSkMatrix(matrix4);
  }

  set_shader(UIDartState::CreateGPUObject(SkGradientShader::MakeRadial(
      SkPoint::Make(center_x, center_y), radius,
      reinterpret_cast<const SkColor*>(colors.data()), color_stops.data(),
      colors.size(), tile_mode, 0, has_matrix ? &sk_matrix : nullptr)));
}

void CanvasGradient::initSweep(double center_x,
                               double center_y,
                               const tonic::Int32List& colors,
                               const tonic::Float32List& color_stops,
                               SkTileMode tile_mode,
                               double start_angle,
                               double end_angle,
                               const tonic::Float64List& matrix4) {
  FML_DCHECK(colors.size() == color_stops.size() ||
             color_stops.data() == nullptr);

  static_assert(sizeof(SkColor) == sizeof(int32_t),
                "SkColor doesn't use int32_t.");

  SkMatrix sk_matrix;
  bool has_matrix = matrix4.data() != nullptr;
  if (has_matrix) {
    sk_matrix = ToSkMatrix(matrix4);
  }

  set_shader(UIDartState::CreateGPUObject(SkGradientShader::MakeSweep(
      center_x, center_y, reinterpret_cast<const SkColor*>(colors.data()),
      color_stops.data(), colors.size(), tile_mode,
      start_angle * 180.0 / M_PI, end_angle * 180.0 / M_PI, 0,
      has_matrix ? &sk_matrix : nullptr)));
}

void CanvasGradient::initTwoPointConical(double start_x,
                                         double start_y,
                                         double start_radius,
                                         double end_x,
                                         double end_y,
                                         double end_radius,
                                         const tonic::Int32List& colors,
                                         const tonic::Float32List& color_stops,
                                         SkTileMode tile_mode,
                                         const tonic::Float64List& matrix4) {
  FML_DCHECK(colors.size() == color_stops.size() ||
             color_stops.data() == nullptr);

  static_assert(sizeof(SkColor) == sizeof(int32_t),
                "SkColor doesn't use int32_t.");

  SkMatrix sk_matrix;
  bool has_matrix = matrix4.data() != nullptr;
  if (has_matrix) {
    sk_matrix = ToSkMatrix(matrix4);
  }

  set_shader(UIDartState::CreateGPUObject(SkGradientShader::MakeTwoPointConical(
      SkPoint::Make(start_x, start_y), start_radius,
      SkPoint::Make(end_x, end_y), end_radius,
      reinterpret_cast<const SkColor*>(colors.data()), color_stops.data(),
      colors.size(), tile_mode, 0, has_matrix ? &sk_matrix : nullptr)));
}

CanvasGradient::CanvasGradient() = default;

CanvasGradient::~CanvasGradient() = default;

}  // namespace flutter
