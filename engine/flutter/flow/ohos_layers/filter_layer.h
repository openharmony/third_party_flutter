// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_OHOS_LAYERS_FILTER_LAYER_H
#define FLUTTER_FLOW_OHOS_LAYERS_FILTER_LAYER_H

#include "third_party/skia/include/core/SkPaint.h"

#include "flutter/flow/ohos_layers/container_layer.h"

namespace flutter::OHOS {

class FilterLayer : public ContainerLayer {
public:
    FilterLayer(const SkPaint& filterPaint) : filterPaint_(filterPaint) {}

    ~FilterLayer() override = default;

    void Paint(const PaintContext& paintContext) const override;

private:
    SkPaint filterPaint_;

    FML_DISALLOW_COPY_AND_ASSIGN(FilterLayer);
};

}  // namespace flutter::OHOS

#endif  // FLUTTER_FLOW_OHOS_LAYERS_FILTER_LAYER_H