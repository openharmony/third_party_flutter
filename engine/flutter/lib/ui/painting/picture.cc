// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/picture.h"

#include "flutter/lib/ui/painting/canvas.h"
#include "third_party/skia/include/core/SkImage.h"
#include "flutter/lib/ui/ui_dart_state.h"

namespace flutter {

fml::RefPtr<Picture> Picture::Create(SkiaGPUObject<SkPicture> picture) {
  return fml::MakeRefCounted<Picture>(std::move(picture));
}

Picture::Picture(SkiaGPUObject<SkPicture> picture)
    : picture_(std::move(picture)) {}

Picture::~Picture() = default;

fml::RefPtr<CanvasImage> Picture::toImage(int width, int height) {
  fml::RefPtr<CanvasImage> image = CanvasImage::Create();
  image->set_image(UIDartState::CreateGPUObject<SkImage>(SkImage::MakeFromPicture(
      picture_.get(), SkISize::Make(width, height), nullptr, nullptr,
      SkImage::BitDepth::kU8, SkColorSpace::MakeSRGB())));
  return image;
}

Dart_Handle Picture::RasterizeToImage(sk_sp<SkPicture> picture,
                                      uint32_t width,
                                      uint32_t height,
                                      Dart_Handle raw_image_callback) {
  return nullptr;
}

size_t Picture::GetAllocationSize() {
  if (auto picture = picture_.get()) {
    return picture->approximateBytesUsed();
  } else {
    return sizeof(Picture);
  }
}

}  // namespace blink
