// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Increase the process used in native_view mode.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#ifndef FLUTTER_FLOW_OHOS_LAYERS_LAYER_TREE_BUILDER_H
#define FLUTTER_FLOW_OHOS_LAYERS_LAYER_TREE_BUILDER_H

#include <cstdint>
#include <memory>
#include <stack>

#include "experimental/svg/model/SkSVGDOM.h"
#include "third_party/skia/include/core/SkImageFilter.h"
#include "third_party/skia/include/core/SkPath.h"
#include "third_party/skia/include/core/SkPaint.h"
#include "third_party/skia/include/core/SkPicture.h"
#include "third_party/skia/include/core/SkRRect.h"
#include "third_party/skia/include/core/SkShader.h"

#include "flutter/flow/ohos_layers/container_layer.h"
#include "flutter/flow/ohos_layers/layer.h"
#include "flutter/flow/ohos_layers/layer_tree.h"

namespace flutter::OHOS {

class LayerTreeBuilder {
public:
    static std::unique_ptr<LayerTreeBuilder> Create()
    {
        return std::make_unique<LayerTreeBuilder>();
    }
    LayerTreeBuilder() = default;
    ~LayerTreeBuilder() = default;

    void AddPicture(double dx, double dy, sk_sp<SkPicture> picture);
    void AddTexture(double dx, double dy, double width, double height, int64_t textureId, bool freeze, uint8_t opacity);
    void PushTransform(const SkMatrix& matrix);
    void PushOffset(double dx, double dy);
    void PushClipRect(double left, double right, double top, double bottom, int32_t clipBehavior);
    void PushClipRRect(const SkRRect& skRRect, int32_t clipBehavior);
    void PushClipPath(const SkPath& skPath, int32_t clipBehavior);
    void PushOpacity(int32_t alpha, double dx = 0, double dy = 0);
    void PushBackdropFilter(sk_sp<SkImageFilter> filter);
    void PushShaderMask(sk_sp<SkShader> shader, double maskRectLeft, double maskRectRight, double maskRectTop,
        double maskRectBottom, int32_t blendMode);
    void PushGradientColorMask(const SkPaint& maskPaint);
    void PushSvgMask(const sk_sp<SkSVGDOM>& svgDom, double x, double y, double scaleX, double scaleY);
    void PushPathMask(const SkPaint& maskPaint, const SkPath& maskPath);
    void PushFilter(const SkPaint& filterPaint);

    void Pop();
    std::unique_ptr<LayerTree> GetLayerTree() const;

private:
    void PushLayer(const std::shared_ptr<ContainerLayer>& layer);

    std::shared_ptr<ContainerLayer> rootLayer_;
    ContainerLayer* currentLayer_ = nullptr;

    FML_DISALLOW_COPY_AND_ASSIGN(LayerTreeBuilder);
};

} // namespace flutter::OHOS

#endif // FLUTTER_FLOW_OHOS_LAYERS_LAYER_TREE_BUILDER_H