// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Increase the process used in native_view mode.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#include "flutter/flow/ohos_layers/layer_tree_builder.h"

#include "third_party/skia/include/core/SkBlendMode.h"
#include "third_party/skia/include/core/SkColorFilter.h"
#include "third_party/skia/include/core/SkMatrix.h"
#include "third_party/skia/include/core/SkPoint.h"
#include "third_party/skia/include/core/SkRefCnt.h"
#include "third_party/skia/include/core/SkSize.h"

#include "flutter/flow/ohos_layers/backdrop_filter_layer.h"
#include "flutter/flow/ohos_layers/clip_path_layer.h"
#include "flutter/flow/ohos_layers/clip_rect_layer.h"
#include "flutter/flow/ohos_layers/clip_rrect_layer.h"
#include "flutter/flow/ohos_layers/color_filter_layer.h"
#include "flutter/flow/ohos_layers/container_layer.h"
#include "flutter/flow/ohos_layers/filter_layer.h"
#include "flutter/flow/ohos_layers/layer.h"
#include "flutter/flow/ohos_layers/mask_layer.h"
#include "flutter/flow/ohos_layers/opacity_layer.h"
#include "flutter/flow/ohos_layers/picture_layer.h"
#include "flutter/flow/ohos_layers/shader_mask_layer.h"
#include "flutter/flow/ohos_layers/texture_layer.h"
#include "flutter/flow/ohos_layers/transform_layer.h"

namespace flutter::OHOS {

void LayerTreeBuilder::PushTransform(const SkMatrix& matrix)
{
    auto layer = std::make_shared<TransformLayer>(matrix);
    PushLayer(layer);
}

void LayerTreeBuilder::PushOffset(double dx, double dy)
{
    SkMatrix skMatrix = SkMatrix::MakeTrans(dx, dy);
    auto layer = std::make_shared<TransformLayer>(skMatrix);
    PushLayer(layer);
}

void LayerTreeBuilder::PushClipRect(double left, double right, double top, double bottom, int32_t clipBehavior)
{
    SkRect clipRect = SkRect::MakeLTRB(left, top, right, bottom);
    Clip clip_behavior = static_cast<Clip>(clipBehavior);
    auto layer = std::make_shared<ClipRectLayer>(clipRect, clip_behavior);
    PushLayer(layer);
}

void LayerTreeBuilder::PushClipRRect(const SkRRect& skRRect, int32_t clipBehavior)
{
    Clip clip_behavior = static_cast<Clip>(clipBehavior);
    auto layer = std::make_shared<ClipRRectLayer>(skRRect, clip_behavior);
    PushLayer(layer);
}

void LayerTreeBuilder::PushClipPath(const SkPath& skPath, int32_t clipBehavior)
{
    Clip clip_behavior = static_cast<Clip>(clipBehavior);
    auto layer = std::make_shared<ClipPathLayer>(skPath, clip_behavior);
    PushLayer(layer);
}

void LayerTreeBuilder::PushOpacity(int32_t alpha, double dx, double dy)
{
    auto layer = std::make_shared<OpacityLayer>(alpha, SkPoint::Make(dx, dy));
    PushLayer(layer);
}

void LayerTreeBuilder::PushBackdropFilter(sk_sp<SkImageFilter> filter)
{
    auto layer = std::make_shared<BackdropFilterLayer>(filter);
    PushLayer(layer);
}

void LayerTreeBuilder::PushShaderMask(sk_sp<SkShader> shader, double maskRectLeft, double maskRectRight,
    double maskRectTop, double maskRectBottom, int32_t blendMode)
{
    SkRect rect = SkRect::MakeLTRB(maskRectLeft, maskRectTop, maskRectRight, maskRectBottom);
    auto layer = std::make_shared<ShaderMaskLayer>(shader, rect, static_cast<SkBlendMode>(blendMode));
    PushLayer(layer);
}

void LayerTreeBuilder::PushSvgMask(const sk_sp<SkSVGDOM>& svgDom, double x, double y, double scaleX, double scaleY)
{
    auto layer = std::make_shared<MaskLayer>(x, y, scaleX, scaleY, svgDom);
    PushLayer(layer);
}

void LayerTreeBuilder::PushGradientColorMask(const SkPaint& maskPaint)
{
    auto layer = std::make_shared<MaskLayer>(maskPaint);
    PushLayer(layer);
}

void LayerTreeBuilder::PushPathMask(const SkPaint& maskPaint, const SkPath& maskPath)
{
    auto layer = std::make_shared<MaskLayer>(maskPaint, maskPath);
    PushLayer(layer);
}

void LayerTreeBuilder::PushFilter(const SkPaint& filterPaint)
{
    auto layer = std::make_shared<FilterLayer>(filterPaint);
    PushLayer(layer);
}

void LayerTreeBuilder::AddPicture(double dx, double dy, sk_sp<SkPicture> picture)
{
    if (currentLayer_ == nullptr) {
        return;
    }
    SkPoint offset = SkPoint::Make(dx, dy);
    SkRect pictureRect = picture->cullRect();
    pictureRect.offset(offset.x(), offset.y());
    auto layer = std::make_shared<PictureLayer>(offset, picture);
    currentLayer_->Add(std::move(layer));
}

void LayerTreeBuilder::AddTexture(double dx, double dy, double width, double height,
    int64_t textureId, bool freeze, uint8_t opacity)
{
    if (currentLayer_ == nullptr) {
        return;
    }
    auto layer = std::make_shared<TextureLayer>(SkPoint::Make(dx, dy), SkSize::Make(width, height), textureId, opacity);
    currentLayer_->Add(std::move(layer));
}

void LayerTreeBuilder::PushLayer(const std::shared_ptr<ContainerLayer>& layer)
{
    if (!rootLayer_) {
        rootLayer_ = layer;
        currentLayer_ = layer.get();
        return;
    }

    if (currentLayer_ == nullptr) {
        return;
    }

    currentLayer_->Add(layer);
    currentLayer_ = layer.get();
}

void LayerTreeBuilder::Pop()
{
    if (currentLayer_ == nullptr) {
        return;
    }
    currentLayer_ = currentLayer_->parent();
}

std::unique_ptr<LayerTree> LayerTreeBuilder::GetLayerTree() const
{
    return std::make_unique<LayerTree>(rootLayer_);
}

} // namespace flutter::OHOS
