// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Increase the process used in native_view mode.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#include "flutter/flow/ohos_layers/texture_layer.h"

#include "flutter/flow/ohos_layers/paint_context.h"

namespace flutter::OHOS {

void TextureLayer::Prepare(const SkMatrix& matrix)
{
    SetPaintBounds(SkRect::MakeXYWH(offset_.x(), offset_.y(), textureSize_.width(), textureSize_.height()));
}

void TextureLayer::Paint(const PaintContext& paintContext) const
{
    paintContext.Paint(textureId_, offset_, opacity_);
}

}  // namespace flutter::OHOS