// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/container_layer.h"

namespace flutter {

ContainerLayer::ContainerLayer() {}

ContainerLayer::~ContainerLayer() = default;

void ContainerLayer::Add(std::shared_ptr<Layer> layer) {
  layer->set_parent(this);
  layers_.push_back(std::move(layer));
}

void ContainerLayer::Preroll(PrerollContext* context, const SkMatrix& matrix) {
  TRACE_EVENT0("flutter", "ContainerLayer::Preroll");

  SkRect child_paint_bounds = SkRect::MakeEmpty();
  PrerollChildren(context, matrix, &child_paint_bounds);
  set_paint_bounds(child_paint_bounds);
}

void ContainerLayer::PrerollChildren(PrerollContext* context,
                                     const SkMatrix& child_matrix,
                                     SkRect* child_paint_bounds) {
  MergeParentHole();
  for (auto& layer : layers_) {
    layer->Preroll(context, child_matrix);

    if (layer->needs_system_composite()) {
      set_needs_system_composite(true);
    }
    child_paint_bounds->join(layer->paint_bounds());
  }
}

void ContainerLayer::PaintChildren(PaintContext& context) const {
  FML_DCHECK(needs_painting());

  // Intentionally not tracing here as there should be no self-time
  // and the trace event on this common function has a small overhead.
  for (auto& layer : layers_) {
    if (layer->needs_painting()) {
      layer->Paint(context);
    }
  }
}

void ContainerLayer::MarkHole(int hole_id, const SkRect& rect) {
  hole_regions_.try_emplace(hole_id, rect);
  SkRect dst_rect = MapRect(rect);
  auto* parent_layer = parent();
  if (parent_layer != nullptr) {
    parent_layer->MarkHole(hole_id, dst_rect);
  }
}

void ContainerLayer::RemoveHole(int hole_id) {
  hole_regions_.erase(hole_id);
  for (auto& layer : layers_) {
    if (layer->IsContainer()) {
      auto* containerLayer = static_cast<ContainerLayer*>(layer.get());
      containerLayer->RemoveHole(hole_id);
    }
  }
}

void ContainerLayer::MergeParentHole() {
  auto* parent_layer = parent();
  if (!parent_layer) {
    return;
  }
  for (const auto& [id, rect] : parent_layer->HoleRegions()) {
    hole_regions_.try_emplace(id, rect);
  }
}

#if defined(OS_FUCHSIA)

void ContainerLayer::UpdateScene(SceneUpdateContext& context) {
  UpdateSceneChildren(context);
}

void ContainerLayer::UpdateSceneChildren(SceneUpdateContext& context) {
  FML_DCHECK(needs_system_composite());

  // Paint all of the layers which need to be drawn into the container.
  // These may be flattened down to a containing
  for (auto& layer : layers_) {
    if (layer->needs_system_composite()) {
      layer->UpdateScene(context);
    }
  }
}

#endif  // defined(OS_FUCHSIA)

}  // namespace flutter
