// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Increase the process used in native_view mode.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#ifndef FLUTTER_FLOW_OHOS_LAYERS_PAINT_CONTEXT_H
#define FLUTTER_FLOW_OHOS_LAYERS_PAINT_CONTEXT_H

#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkPoint.h"

#include "flutter/flow/ohos_layers/texture_register.h"

namespace flutter::OHOS {

struct PaintContext {
  PaintContext(SkCanvas* skCvs,
               const std::shared_ptr<flutter::OHOS::TextureRegistry>& texReg)
      : textureRegistry(texReg), skCanvas(skCvs) {}
  virtual ~PaintContext() = default;
  virtual void Paint(int64_t textureId, const SkPoint& offset, uint8_t opacity) const = 0;
  std::shared_ptr<flutter::OHOS::TextureRegistry> textureRegistry;
  SkCanvas* skCanvas = nullptr;
};

}  // namespace flutter::OHOS

#endif  // FOUNDATION_ACE_CORE_RENDERING_PAINT_CONTEXT_H
