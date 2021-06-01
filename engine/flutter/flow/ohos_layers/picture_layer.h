// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Increase the process used in native_view mode.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#ifndef FLUTTER_FLOW_OHOS_LAYERS_PICTURE_LAYER_H
#define FLUTTER_FLOW_OHOS_LAYERS_PICTURE_LAYER_H

#include <memory>

#include "third_party/skia/include/core/SkPicture.h"

#include "flutter/flow/ohos_layers/layer.h"

namespace flutter::OHOS {

class PictureLayer : public Layer {
public:
    PictureLayer(const SkPoint& offset, sk_sp<SkPicture> picture)
        : offset_(offset), picture_(std::move(picture)) {}
    ~PictureLayer() override = default;

    SkPicture* GetPicture() const
    {
        return picture_.get();
    }

    void Prepare(const SkMatrix& matrix) override;

    void Paint(const PaintContext& paintContext) const override;

private:
    SkPoint offset_;
    sk_sp<SkPicture> picture_;

    FML_DISALLOW_COPY_AND_ASSIGN(PictureLayer);
};

}  // namespace flutter::OHOS

#endif  // FLUTTER_FLOW_OHOS_LAYERS_PICTURE_LAYER_H