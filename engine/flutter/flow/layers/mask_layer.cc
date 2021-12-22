// Copyright 2021 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/mask_layer.h"

#include "third_party/skia/include/core/SkMaskFilter.h"
#include "third_party/skia/include/effects/SkLumaColorFilter.h"

namespace flutter {

void MaskLayer::Paint(PaintContext& context) const {
  TRACE_EVENT0("flutter", "MaskLayer::Paint");
  FML_DCHECK(needs_painting());

  if (isSvgMask_ && !svgDom_) {
    return;
  }

  auto& skCanvas = context.internal_nodes_canvas;
  SkAutoCanvasRestore save(skCanvas, true);
  SkRect maskBounds = paint_bounds();

  skCanvas->saveLayer(maskBounds, nullptr);
  int tmpLayer = skCanvas->getSaveCount();

  SkPaint mask_filter;
#ifdef USE_SYSTEM_SKIA
  auto filter = SkColorFilter::MakeComposeFilter(SkLumaColorFilter::Make(),
      SkColorFilter::MakeSRGBToLinearGamma());
#else
  auto filter = SkColorFilters::Compose(SkLumaColorFilter::Make(),
      SkColorFilters::SRGBToLinearGamma());
#endif
  mask_filter.setColorFilter(filter);
  skCanvas->saveLayer(maskBounds, &mask_filter);
  if (isSvgMask_) {
    SkAutoCanvasRestore maskSave(skCanvas, true);
    skCanvas->translate(maskBounds.fLeft + svgX_, maskBounds.fTop + svgY_);
    skCanvas->scale(scaleX_, scaleY_);
    svgDom_->render(skCanvas);
  } else if (isGradientMask_) {
    SkAutoCanvasRestore maskSave(skCanvas, true);
    skCanvas->translate(maskBounds.fLeft, maskBounds.fTop);
    SkRect skRect = SkRect::MakeIWH(maskBounds.fRight - maskBounds.fLeft, maskBounds.fBottom - maskBounds.fTop);
    skCanvas->drawRect(skRect, maskPaint_);
  } else if (isPathMask_) {
    SkAutoCanvasRestore maskSave(skCanvas, true);
    skCanvas->translate(maskBounds.fLeft, maskBounds.fTop);
    skCanvas->drawPath(maskPath_, maskPaint_);
  }

  // back to mask layer
  skCanvas->restoreToCount(tmpLayer);
  // create content layer
  SkPaint maskPaint;
  maskPaint.setBlendMode(SkBlendMode::kSrcIn);
  skCanvas->saveLayer(maskBounds, &maskPaint);
  skCanvas->clipRect(maskBounds, true);
  PaintChildren(context);
}

}  // namespace flutter
