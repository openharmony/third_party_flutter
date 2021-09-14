// Copyright 2021 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LAYERS_FILTER_LAYER_H_
#define FLUTTER_FLOW_LAYERS_FILTER_LAYER_H_

#include "flutter/flow/layers/container_layer.h"

#include "third_party/skia/include/core/SkPaint.h"

namespace flutter {

class FilterLayer : public ContainerLayer {
public:
  FilterLayer(const SkPaint& filterPaint) : filterPaint_(filterPaint) {}
  ~FilterLayer() override = default;

  void Paint(PaintContext& context) const override;

private:
  SkPaint filterPaint_;

  FML_DISALLOW_COPY_AND_ASSIGN(FilterLayer);
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_LAYERS_FILTER_LAYER_H_
