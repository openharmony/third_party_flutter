// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Increase the process used in native_view mode.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#ifndef FLUTTER_FLOW_OHOS_LAYERS_CONTAINER_LAYER_H
#define FLUTTER_FLOW_OHOS_LAYERS_CONTAINER_LAYER_H

#include <vector>

#include "flutter/flow/ohos_layers/layer.h"

namespace flutter::OHOS {

class ContainerLayer : public Layer {
public:
    ContainerLayer() = default;
    ~ContainerLayer() override = default;

    void Add(const std::shared_ptr<Layer>& layer);

    const std::vector<std::shared_ptr<Layer>>& GetLayers() const
    {
        return layers_;
    }

    void Prepare(const SkMatrix& matrix) override;

protected:
    void PaintChildren(const PaintContext& paintContext) const;

    void PrepareChildren(const SkMatrix& childMatrix, SkRect& childPaintBounds);

    void ClearChildren()
    {
        layers_.clear();
    }

private:
    std::vector<std::shared_ptr<Layer>> layers_;

    FML_DISALLOW_COPY_AND_ASSIGN(ContainerLayer);
};

} // namespace flutter::OHOS

#endif // FLUTTER_FLOW_OHOS_LAYERS_CONTAINER_LAYER_H