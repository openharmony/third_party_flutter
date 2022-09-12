// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LAYERS_HOLE_LAYER_H_
#define FLUTTER_FLOW_LAYERS_HOLE_LAYER_H_

#include "flutter/flow/layers/container_layer.h"
#include "include/core/SkRect.h"

namespace flutter {

class HoleLayer : public ContainerLayer {
 public:

  HoleLayer(const SkRect& rect, int hole_id);
  ~HoleLayer() override;

  void Preroll(PrerollContext* frame, const SkMatrix& matrix) override;
  void Paint(PaintContext& context) const override;
  void MarkHole();

 private:
  SkRect rect_;
  int hole_id_;

  FML_DISALLOW_COPY_AND_ASSIGN(HoleLayer);
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_LAYERS_HOLE_LAYER_H_
