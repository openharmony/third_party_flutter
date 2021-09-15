// Copyright 2021 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LAYERS_MASK_LAYER_H_
#define FLUTTER_FLOW_LAYERS_MASK_LAYER_H_

#include "flutter/flow/layers/container_layer.h"

#include "experimental/svg/model/SkSVGDOM.h"
#include "third_party/skia/include/core/SkPaint.h"
#include "third_party/skia/include/core/SkPath.h"

namespace flutter {

class MaskLayer : public ContainerLayer {
 public:
  MaskLayer(const SkPaint& maskPaint)
      : isGradientMask_(true), maskPaint_(maskPaint) {}
  MaskLayer(double x, double y, double scaleX, double scaleY, const sk_sp<SkSVGDOM>& svgDom)
      : isSvgMask_(true), svgX_(x), svgY_(y), scaleX_(scaleX), scaleY_(scaleY), svgDom_(svgDom) {}
  MaskLayer(const SkPaint& maskPaint, const SkPath& maskPath)
      : isPathMask_(true), maskPaint_(maskPaint), maskPath_(maskPath) {}
  ~MaskLayer() override = default;

  void Paint(PaintContext& context) const override;

 private:
  bool isSvgMask_ = false;
  bool isGradientMask_ = false;
  bool isPathMask_ = false;
  double svgX_ = 0.0f;
  double svgY_ = 0.0f;
  double scaleX_ = 1.0f;
  double scaleY_ = 1.0f;
  sk_sp<SkSVGDOM> svgDom_;
  SkPaint maskPaint_;
  SkPath maskPath_;

  FML_DISALLOW_COPY_AND_ASSIGN(MaskLayer);
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_LAYERS_MASK_LAYER_H_
