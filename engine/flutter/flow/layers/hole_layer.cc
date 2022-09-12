// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/hole_layer.h"

namespace flutter {

HoleLayer::HoleLayer(const SkRect& rect, int hole_id)
    : rect_(rect), hole_id_(hole_id) {}

HoleLayer::~HoleLayer() = default;

void HoleLayer::Preroll(PrerollContext* context, const SkMatrix& matrix)
{
  if (context->cull_rect.intersect(rect_)) {
    SkRect child_paint_bounds = SkRect::MakeEmpty();
    PrerollChildren(context, matrix, &child_paint_bounds);
    set_paint_bounds(rect_);
  }
}

void HoleLayer::Paint(PaintContext& context) const {
  TRACE_EVENT0("flutter", "HoleLayer::Paint");
  auto* root_layer = parent();
  if (!root_layer) {
    return;
  }
  while (root_layer->parent() != nullptr) {
    root_layer = root_layer->parent();
  }
  root_layer->RemoveHole(hole_id_);
}

void HoleLayer::MarkHole() {
  auto* parent_layer = parent();
  if (parent_layer != nullptr) {
    parent_layer->MarkHole(hole_id_, rect_);
  }
}

}  // namespace flutter
