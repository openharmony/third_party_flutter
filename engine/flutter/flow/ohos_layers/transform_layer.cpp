// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Increase the process used in native_view mode.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#include "flutter/flow/ohos_layers/transform_layer.h"

#include "flutter/flow/ohos_layers/paint_context.h"

namespace flutter::OHOS {

void TransformLayer::Prepare(const SkMatrix& matrix)
{
    SkMatrix childMatrix;
    childMatrix.setConcat(matrix, transform_);
    SkRect childPaintBounds = SkRect::MakeEmpty();
    PrepareChildren(childMatrix, childPaintBounds);
    transform_.mapRect(&childPaintBounds);
    SetPaintBounds(childPaintBounds);
}

void TransformLayer::Paint(const PaintContext& paintContext) const
{
    SkAutoCanvasRestore save(paintContext.skCanvas, true);
    paintContext.skCanvas->concat(transform_);
    PaintChildren(paintContext);
}

}  // namespace flutter::OHOS