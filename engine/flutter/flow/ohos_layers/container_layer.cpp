// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Increase the process used in native_view mode.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#include "flutter/flow/ohos_layers/container_layer.h"

#include "flutter/flow/ohos_layers/paint_context.h"

namespace flutter::OHOS {

void ContainerLayer::Add(const std::shared_ptr<Layer>& layer)
{
    layer->setParent(this);
    layers_.push_back(layer);
}

void ContainerLayer::Prepare(const SkMatrix& matrix)
{
    SkRect childPaintBounds = SkRect::MakeEmpty();
    PrepareChildren(matrix, childPaintBounds);
    SetPaintBounds(childPaintBounds);
}

void ContainerLayer::PrepareChildren(const SkMatrix& childMatrix, SkRect& childPaintBounds)
{
    for (auto& layer : layers_) {
        layer->Prepare(childMatrix);
        childPaintBounds.join(layer->GetPaintBounds());
    }
}

void ContainerLayer::PaintChildren(const PaintContext& paintContext) const
{
    for (auto& layer : layers_) {
        layer->Paint(paintContext);
    }
}

} // namespace flutter::OHOS