// Copyright 2021 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/filter_layer.h"

namespace flutter {

void FilterLayer::Paint(PaintContext& context) const {
  TRACE_EVENT0("flutter", "FilterLayer::Paint");
  FML_DCHECK(needs_painting());

  auto& skCanvas = context.internal_nodes_canvas;
  SkAutoCanvasRestore save(skCanvas, true);
  skCanvas->saveLayer(nullptr, &filterPaint_);

  PaintChildren(context);
}

}  // namespace flutter
