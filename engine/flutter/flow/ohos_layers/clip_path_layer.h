// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Increase the process used in native_view mode.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#ifndef FLUTTER_FLOW_OHOS_LAYERS_CLIP_PATH_LAYER_H
#define FLUTTER_FLOW_OHOS_LAYERS_CLIP_PATH_LAYER_H

#include "third_party/skia/include/core/SkPath.h"

#include "flutter/flow/ohos_layers/container_layer.h"

namespace flutter::OHOS {

class ClipPathLayer : public ContainerLayer {
public:
    ClipPathLayer(const SkPath& clipPath, Clip clipBehavior) : clipPath_(clipPath), clipBehavior_(clipBehavior) {}
    ~ClipPathLayer() override = default;

    void Prepare(const SkMatrix& matrix) override;

    void Paint(const PaintContext& paintContext) const override;

private:
    SkPath clipPath_;
    Clip clipBehavior_;

    FML_DISALLOW_COPY_AND_ASSIGN(ClipPathLayer);
};

} // namespace flutter::OHOS

#endif // FLUTTER_FLOW_OHOS_LAYERS_CLIP_PATH_LAYER_H