// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Increase the process used in native_view mode.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#include "flutter/flow/ohos_layers/opacity_layer.h"

#include "flutter/flow/ohos_layers/paint_context.h"
#include "flutter/flow/ohos_layers/transform_layer.h"

namespace  flutter::OHOS {

void OpacityLayer::Prepare(const SkMatrix& matrix)
{
    SkMatrix childMatrix = matrix;
    childMatrix.postTranslate(offset_.fX, offset_.fY);
    ContainerLayer::Prepare(childMatrix);
    SetPaintBounds(GetPaintBounds().makeOffset(offset_.fX, offset_.fY));
}

void OpacityLayer::Paint(const PaintContext& paintContext) const
{
    SkPaint paint;
    paint.setAlpha(alpha_);
    SkAutoCanvasRestore save(paintContext.skCanvas, true);
    paintContext.skCanvas->translate(offset_.fX, offset_.fY);
    SkRect saveLayerBounds;
    GetPaintBounds().makeOffset(-offset_.fX, -offset_.fY).roundOut(&saveLayerBounds);
    paintContext.skCanvas->saveLayer(saveLayerBounds, &paint);
    PaintChildren(paintContext);
    paintContext.skCanvas->restore();
}

}  // namespace  flutter::OHOS