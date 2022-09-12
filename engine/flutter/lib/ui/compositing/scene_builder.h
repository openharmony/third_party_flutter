// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_COMPOSITING_SCENE_BUILDER_H_
#define FLUTTER_LIB_UI_COMPOSITING_SCENE_BUILDER_H_

#include <stdint.h>

#include <memory>
#include <stack>

#include "flutter/lib/ui/compositing/scene.h"
#include "flutter/lib/ui/dart_wrapper.h"
#include "flutter/lib/ui/painting/color_filter.h"
#include "flutter/lib/ui/painting/engine_layer.h"
#include "flutter/lib/ui/painting/image_filter.h"
#include "flutter/lib/ui/painting/path.h"
#include "flutter/lib/ui/painting/picture.h"
#include "flutter/lib/ui/painting/rrect.h"
#include "flutter/lib/ui/painting/shader.h"
#include "flutter/lib/ui/ui_dart_state.h"

#if defined(OS_FUCHSIA)
#include "flutter/lib/ui/compositing/scene_host.h"
#endif

#include "experimental/svg/model/SkSVGDOM.h"
#include "third_party/skia/include/core/SkPaint.h"
#include "third_party/skia/include/core/SkPath.h"

namespace flutter {

class SceneBuilder : public RefCountedDartWrappable<SceneBuilder> {
  DEFINE_WRAPPERTYPEINFO();
  FML_FRIEND_MAKE_REF_COUNTED(SceneBuilder);

 public:
  static fml::RefPtr<SceneBuilder> create() {
    return fml::MakeRefCounted<SceneBuilder>();
  }

  ~SceneBuilder() override;

  fml::RefPtr<EngineLayer> pushTransform(tonic::Float64List& matrix4);
  fml::RefPtr<EngineLayer> pushOffset(double dx, double dy);
  fml::RefPtr<EngineLayer> pushClipRect(double left,
                                        double right,
                                        double top,
                                        double bottom,
                                        int clipBehavior);
  fml::RefPtr<EngineLayer> pushClipRRect(const RRect& rrect, int clipBehavior);
  fml::RefPtr<EngineLayer> pushClipPath(const CanvasPath* path,
                                        int clipBehavior);
  fml::RefPtr<EngineLayer> pushOpacity(int alpha, double dx = 0, double dy = 0);
  fml::RefPtr<EngineLayer> pushColorFilter(const ColorFilter* color_filter);
  fml::RefPtr<EngineLayer> pushBackdropFilter(ImageFilter* filter);
  fml::RefPtr<EngineLayer> pushShaderMask(Shader* shader,
                                          double maskRectLeft,
                                          double maskRectRight,
                                          double maskRectTop,
                                          double maskRectBottom,
                                          int blendMode);
  fml::RefPtr<EngineLayer> pushPhysicalShape(const CanvasPath* path,
                                             double elevation,
                                             int color,
                                             int shadowColor,
                                             int clipBehavior);
  fml::RefPtr<EngineLayer> PushGradientColorMask(const SkPaint& maskPaint);
  fml::RefPtr<EngineLayer> PushSvgMask(const sk_sp<SkSVGDOM>& svgDom, double x, double y, double scaleX, double scaleY);
  fml::RefPtr<EngineLayer> PushPathMask(const SkPaint& maskPaint, const SkPath& maskPath);
  fml::RefPtr<EngineLayer> PushFilter(const SkPaint& filterPaint);
  fml::RefPtr<EngineLayer> PushHole(const SkRect& rect, int32_t hole_id);

  void addRetained(fml::RefPtr<EngineLayer> retainedLayer);

  void pop();

  void addPerformanceOverlay(uint64_t enabledOptions,
                             double left,
                             double right,
                             double top,
                             double bottom);

  void addPicture(double dx, double dy, Picture* picture, int hints);

  void addTexture(double dx,
                  double dy,
                  double width,
                  double height,
                  int64_t textureId,
                  bool freeze,
                  uint8_t opacity);

  void addPlatformView(double dx,
                       double dy,
                       double width,
                       double height,
                       int64_t viewId);

#if defined(OS_FUCHSIA)
  void addChildScene(double dx,
                     double dy,
                     double width,
                     double height,
                     SceneHost* sceneHost,
                     bool hitTestable);
#endif

  void setRasterizerTracingThreshold(uint32_t frameInterval);

  void setCheckerboardRasterCacheImages(bool checkerboard);
  void setCheckerboardOffscreenLayers(bool checkerboard);

  fml::RefPtr<Scene> build();

  static void RegisterNatives(tonic::DartLibraryNatives* natives);

 private:
  SceneBuilder();

  std::shared_ptr<flutter::ContainerLayer> root_layer_;
  flutter::ContainerLayer* current_layer_ = nullptr;

  int rasterizer_tracing_threshold_ = 0;
  bool checkerboard_raster_cache_images_ = false;
  bool checkerboard_offscreen_layers_ = false;

  void PushLayer(std::shared_ptr<flutter::ContainerLayer> layer);

  FML_DISALLOW_COPY_AND_ASSIGN(SceneBuilder);
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_COMPOSITING_SCENE_BUILDER_H_
