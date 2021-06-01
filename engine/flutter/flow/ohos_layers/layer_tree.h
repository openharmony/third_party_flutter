// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Increase the process used in native_view mode.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#ifndef FLUTTER_FLOW_OHOS_LAYERS_LAYER_TREE_H
#define FLUTTER_FLOW_OHOS_LAYERS_LAYER_TREE_H

#include <memory>
#include <stdint.h>

#include "third_party/skia/include/core/SkPicture.h"
#include "third_party/skia/include/core/SkSize.h"

#include "flutter/flow/ohos_layers/layer.h"

namespace flutter::OHOS {

class LayerTree {
public:
    explicit LayerTree(const std::shared_ptr<Layer>& rootLayer) : rootLayer_(rootLayer) {}
    ~LayerTree() = default;

    std::shared_ptr<Layer> GetRootLayer() const
    {
        return rootLayer_;
    }

    const SkISize& GetFrameSize() const
    {
        return treeSize_;
    }

    void SetFrameSize(const SkISize& treeSize)
    {
        treeSize_ = treeSize;
    }

    void Prepare();

private:
    SkISize treeSize_ { 0, 0 }; // Physical pixels.
    std::shared_ptr<Layer> rootLayer_;

    FML_DISALLOW_COPY_AND_ASSIGN(LayerTree);
};

} // namespace flutter::OHOS

#endif // FLUTTER_FLOW_OHOS_LAYERS_LAYER_TREE_H