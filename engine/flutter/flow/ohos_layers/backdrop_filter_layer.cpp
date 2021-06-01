// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Increase the process used in native_view mode.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#include "flutter/flow/ohos_layers/backdrop_filter_layer.h"

#include "third_party/skia/include/core/SkCanvas.h"

#include "flutter/flow/ohos_layers/paint_context.h"

namespace flutter::OHOS {

void BackdropFilterLayer::Paint(const PaintContext& paintContext) const
{
    paintContext.skCanvas->saveLayer(SkCanvas::SaveLayerRec { &GetPaintBounds(), nullptr, imageFilter_.get(), 0 });
    PaintChildren(paintContext);
    paintContext.skCanvas->restore();
}

} // namespace flutter::OHOS