// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/ohos_layers/filter_layer.h"

#include "flutter/flow/ohos_layers/paint_context.h"

namespace flutter::OHOS {

void FilterLayer::Paint(const PaintContext& paintContext) const
{
    SkAutoCanvasRestore save(paintContext.skCanvas, true);
    auto& skCanvas_ = paintContext.skCanvas;
    skCanvas_->saveLayer(nullptr, &filterPaint_);
    PaintChildren(paintContext);
}

} // namespace flutter::OHOS