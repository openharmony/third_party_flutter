// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_OHOS_LAYERS_MASK_LAYER_H
#define FLUTTER_FLOW_OHOS_LAYERS_MASK_LAYER_H

#include "experimental/svg/model/SkSVGDOM.h"
#include "third_party/skia/include/core/SkPaint.h"
#include "third_party/skia/include/core/SkPath.h"
#include "third_party/skia/include/core/SkStream.h"

#include "flutter/flow/ohos_layers/container_layer.h"
#include "flutter/flow/ohos_layers/layer.h"

namespace flutter::OHOS {

class MaskLayer : public ContainerLayer {
public:
    MaskLayer(const SkPaint& maskPaint)
        : isGradientMask_(true), maskPaint_(maskPaint) {}

    MaskLayer(double x, double y, double scaleX, double scaleY, const sk_sp<SkSVGDOM>& svgDom)
        : isSvgMask_(true), svgX_(x), svgY_(y), scaleX_(scaleX), scaleY_(scaleY), svgDom_(svgDom) {}

    MaskLayer(const SkPaint& maskPaint, const SkPath& maskPath)
        : isPathMask_(true), maskPaint_(maskPaint), maskPath_(maskPath) {}

    ~MaskLayer() override = default;

    void Paint(const PaintContext& paintContext) const override;

private:
    bool isSvgMask_ = false;
    bool isGradientMask_ = false;
    bool isPathMask_ = false;
    double svgX_ = 0.0f;
    double svgY_ = 0.0f;
    double scaleX_ = 1.0f;
    double scaleY_ = 1.0f;
    sk_sp<SkSVGDOM> svgDom_;
    SkPaint maskPaint_;
    SkPath maskPath_;

    FML_DISALLOW_COPY_AND_ASSIGN(MaskLayer);
};

}  // namespace flutter::OHOS

#endif  // FLUTTER_FLOW_OHOS_LAYERS_MASK_LAYER_H