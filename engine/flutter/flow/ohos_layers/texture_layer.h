// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Increase the process used in native_view mode.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#ifndef FLUTTER_FLOW_OHOS_LAYERS_TEXTURE_LAYER_H
#define FLUTTER_FLOW_OHOS_LAYERS_TEXTURE_LAYER_H

#include "third_party/skia/include/core/SkPoint.h"
#include "third_party/skia/include/core/SkSize.h"

#include "flutter/flow/ohos_layers/layer.h"

namespace flutter::OHOS {

constexpr int64_t INVALID_TEXTURE_ID = -1;

class TextureLayer : public Layer {
public:
    TextureLayer(const SkPoint& offset, SkSize textureSize, int64_t textureId, uint8_t opacity)
        : offset_(offset), textureSize_(textureSize), textureId_(textureId), opacity_(opacity) {}
    ~TextureLayer() override = default;

    void Prepare(const SkMatrix& matrix) override;

    void Paint(const PaintContext& paintContext) const override;

private:
    SkPoint offset_;
    SkSize textureSize_;
    int64_t textureId_ { INVALID_TEXTURE_ID };
    uint8_t opacity_ { UINT8_MAX };

    FML_DISALLOW_COPY_AND_ASSIGN(TextureLayer);
};

}  // namespace flutter::OHOS

#endif  // FLUTTER_FLOW_OHOS_LAYERS_TEXTURE_LAYER_H
