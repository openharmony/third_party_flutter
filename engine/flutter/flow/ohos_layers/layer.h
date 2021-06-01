// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Increase the process used in native_view mode.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#ifndef FLUTTER_FLOW_OHOS_LAYERS_LAYER_H
#define FLUTTER_FLOW_OHOS_LAYERS_LAYER_H

#include "third_party/skia/include/core/SkRRect.h"
#include "third_party/skia/include/core/SkRect.h"

#include "flutter/fml/macros.h"

namespace flutter::OHOS {

constexpr uint32_t DEAFAULT_UNIQUEID = -1;

enum Clip { NONE, HARDEDGE, ANTIALIAS, ANTIALIAS_WITH_SAVELAYER };

class ContainerLayer;
struct PaintContext;

class Layer {
public:
    Layer() = default;
    virtual ~Layer() = default;

    virtual void Paint(const PaintContext& paintContext) const {};

    virtual void Prepare(const SkMatrix& matrix) {};

    ContainerLayer* parent() const
    {
        return parent_;
    }

    void setParent(ContainerLayer* parent)
    {
        parent_ = parent;
    }
    bool GetNeedsSystemComposite() const
    {
        return needsSystemComposite_;
    }

    void SetNeedsSystemComposite(bool value)
    {
        needsSystemComposite_ = value;
    }

    const SkRect& GetPaintBounds() const
    {
        return paintBounds_;
    }

    void SetPaintBounds(const SkRect& paintBounds)
    {
        paintBounds_ = paintBounds;
    }

    bool NeedsPainting() const
    {
        return !paintBounds_.isEmpty();
    }

    uint64_t GetUniqueId() const
    {
        return uniqueId_;
    }

private:
    ContainerLayer* parent_;
    SkRect paintBounds_;
    bool needsSystemComposite_ = false;
    uint64_t uniqueId_ = DEAFAULT_UNIQUEID;

    FML_DISALLOW_COPY_AND_ASSIGN(Layer);
};

}  // namespace flutter::OHOS

#endif // FLUTTER_FLOW_OHOS_LAYERS_LAYER_H