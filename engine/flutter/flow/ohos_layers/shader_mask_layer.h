// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Increase the process used in native_view mode.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#ifndef FLUTTER_FLOW_OHOS_LAYERS_SHADER_MASK_LAYER_H
#define FLUTTER_FLOW_OHOS_LAYERS_SHADER_MASK_LAYER_H

#include "third_party/skia/include/core/SkBlendMode.h"
#include "third_party/skia/include/core/SkShader.h"

#include "flutter/flow/ohos_layers/container_layer.h"

namespace flutter::OHOS {

class ShaderMaskLayer : public ContainerLayer {
public:
    ShaderMaskLayer(sk_sp<SkShader> shader, const SkRect& maskRect, SkBlendMode blendMode)
        : shader_(shader), maskRect_(maskRect), blendMode_(blendMode) {}
    ~ShaderMaskLayer() override = default;

    void Paint(const PaintContext& paintContext) const override;

private:
    sk_sp<SkShader> shader_;
    SkRect maskRect_;
    SkBlendMode blendMode_;

    FML_DISALLOW_COPY_AND_ASSIGN(ShaderMaskLayer);
};

}  // namespace flutter::OHOS

#endif  // FLUTTER_FLOW_OHOS_LAYERS_SHADER_MASK_LAYER_H