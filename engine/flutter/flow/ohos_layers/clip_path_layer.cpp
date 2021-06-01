// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Increase the process used in native_view mode.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#include "flutter/flow/ohos_layers/clip_path_layer.h"

#include "flutter/flow/ohos_layers/paint_context.h"

namespace flutter::OHOS {

void ClipPathLayer::Prepare(const SkMatrix& matrix)
{
    SkRect clipPathBounds = clipPath_.getBounds();
    SkRect childPaintBounds = SkRect::MakeEmpty();
    PrepareChildren(matrix, childPaintBounds);

    if (childPaintBounds.intersect(clipPathBounds)) {
        SetPaintBounds(childPaintBounds);
    }
}

void ClipPathLayer::Paint(const PaintContext& paintContext) const
{
    SkAutoCanvasRestore save(paintContext.skCanvas, true);
    paintContext.skCanvas->clipPath(clipPath_, clipBehavior_ != Clip::HARDEDGE);

    if (clipBehavior_ == Clip::ANTIALIAS_WITH_SAVELAYER) {
        paintContext.skCanvas->saveLayer(GetPaintBounds(), nullptr);
    }
    PaintChildren(paintContext);
    if (clipBehavior_ == Clip::ANTIALIAS_WITH_SAVELAYER) {
        paintContext.skCanvas->restore();
    }
}

} //  namespace flutter::OHOS