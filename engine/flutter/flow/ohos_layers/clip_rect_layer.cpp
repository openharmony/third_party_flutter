// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Increase the process used in native_view mode.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#include "flutter/flow/ohos_layers/clip_rect_layer.h"

#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkRRect.h"

#include "flutter/flow/ohos_layers/paint_context.h"

namespace flutter::OHOS {

void ClipRectLayer::Prepare(const SkMatrix& matrix)
{
    SkRect childPaintBounds = SkRect::MakeEmpty();
    PrepareChildren(matrix, childPaintBounds);

    if (childPaintBounds.intersect(clipRect_)) {
        SetPaintBounds(childPaintBounds);
    }
}

void ClipRectLayer::Paint(const PaintContext& paintContext) const
{
    SkAutoCanvasRestore save(paintContext.skCanvas, true);
    paintContext.skCanvas->clipRect(clipRect_, clipBehavior_ != Clip::HARDEDGE);

    if (clipBehavior_ == Clip::ANTIALIAS_WITH_SAVELAYER) {
        paintContext.skCanvas->saveLayer(GetPaintBounds(), nullptr);
    }
    PaintChildren(paintContext);
    if (clipBehavior_ == Clip::ANTIALIAS_WITH_SAVELAYER) {
        paintContext.skCanvas->restore();
    }
}

} // namespace flutter::OHOS