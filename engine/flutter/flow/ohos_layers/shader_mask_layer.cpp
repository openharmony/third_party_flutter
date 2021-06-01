// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Increase the process used in native_view mode.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#include "flutter/flow/ohos_layers/shader_mask_layer.h"

#include "flutter/flow/ohos_layers/paint_context.h"

namespace flutter::OHOS {

void ShaderMaskLayer::Paint(const PaintContext& paintContext) const
{
    paintContext.skCanvas->saveLayer(GetPaintBounds(), nullptr);
    PaintChildren(paintContext);

    SkPaint paint;
    paint.setBlendMode(blendMode_);
    paint.setShader(shader_);
    paintContext.skCanvas->translate(maskRect_.left(), maskRect_.top());
    paintContext.skCanvas->drawRect(SkRect::MakeWH(maskRect_.width(), maskRect_.height()), paint);
    paintContext.skCanvas->restore();
}

}  // namespace flutter::OHOS