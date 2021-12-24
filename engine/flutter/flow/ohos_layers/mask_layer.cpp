// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/ohos_layers/mask_layer.h"

#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkMaskFilter.h"
#include "third_party/skia/include/effects/SkLumaColorFilter.h"

#include "flutter/flow/ohos_layers/paint_context.h"

namespace flutter::OHOS {

void MaskLayer::Paint(const PaintContext& paintContext) const
{
    if (isSvgMask_ && !svgDom_) {
        return;
    }

    SkAutoCanvasRestore save(paintContext.skCanvas, true);
    SkRect maskBounds = GetPaintBounds();
    auto& skCanvas_ = paintContext.skCanvas;
    skCanvas_->saveLayer(maskBounds, nullptr);
    int tmpLayer = skCanvas_->getSaveCount();

    SkPaint mask_filter;
#ifdef USE_SYSTEM_SKIA
    auto filter = SkColorFilter::MakeComposeFilter(SkLumaColorFilter::Make(),
        SkColorFilter::MakeSRGBToLinearGamma());
#else
    auto filter = SkColorFilters::Compose(SkLumaColorFilter::Make(),
        SkColorFilters::SRGBToLinearGamma());
#endif
    mask_filter.setColorFilter(filter);
    skCanvas_->saveLayer(maskBounds, &mask_filter);
    if (isSvgMask_) {
        SkAutoCanvasRestore maskSave(paintContext.skCanvas, true);
        skCanvas_->translate(maskBounds.fLeft + svgX_, maskBounds.fTop + svgY_);
        skCanvas_->scale(scaleX_, scaleY_);
        svgDom_->render(skCanvas_);
    } else if (isGradientMask_) {
        SkAutoCanvasRestore maskSave(paintContext.skCanvas, true);
        skCanvas_->translate(maskBounds.fLeft, maskBounds.fTop);
        SkRect skRect = SkRect::MakeIWH(maskBounds.fRight - maskBounds.fLeft, maskBounds.fBottom - maskBounds.fTop);
        skCanvas_->drawRect(skRect, maskPaint_);
    } else if (isPathMask_) {
        SkAutoCanvasRestore maskSave(paintContext.skCanvas, true);
        skCanvas_->translate(maskBounds.fLeft, maskBounds.fTop);
        skCanvas_->drawPath(maskPath_, maskPaint_);
    }

    // back to mask layer
    skCanvas_->restoreToCount(tmpLayer);
    // create content layer
    SkPaint maskPaint;
    maskPaint.setBlendMode(SkBlendMode::kSrcIn);
    skCanvas_->saveLayer(maskBounds, &maskPaint);
    skCanvas_->clipRect(maskBounds, true);
    PaintChildren(paintContext);
}

} // namespace flutter::OHOS