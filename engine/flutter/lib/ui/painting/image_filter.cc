// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/image_filter.h"

#include "flutter/lib/ui/painting/matrix.h"
#include "third_party/skia/include/effects/SkBlurImageFilter.h"
#include "third_party/skia/include/effects/SkImageSource.h"
#include "third_party/skia/include/effects/SkPictureImageFilter.h"

namespace flutter {

IMPLEMENT_WRAPPERTYPEINFO(ui, ImageFilter);

#define FOR_EACH_BINDING(V)   \
  V(ImageFilter, initImage)   \
  V(ImageFilter, initPicture) \
  V(ImageFilter, initBlur)    \
  V(ImageFilter, initMatrix)

FOR_EACH_BINDING(DART_NATIVE_CALLBACK)

void ImageFilter::RegisterNatives(tonic::DartLibraryNatives* natives) {
}

fml::RefPtr<ImageFilter> ImageFilter::Create() {
  return fml::MakeRefCounted<ImageFilter>();
}

ImageFilter::ImageFilter() {}

ImageFilter::~ImageFilter() {}

void ImageFilter::initImage(CanvasImage* image) {
  filter_ = SkImageSource::Make(image->image());
}

void ImageFilter::initPicture(Picture* picture) {
  filter_ = SkPictureImageFilter::Make(picture->picture());
}

void ImageFilter::initBlur(double sigma_x, double sigma_y) {
  filter_ = SkBlurImageFilter::Make(sigma_x, sigma_y, nullptr, nullptr,
                                    SkBlurImageFilter::kClamp_TileMode);
}

void ImageFilter::initMatrix(const std::vector<double>& matrix4,
                             int filterQuality) {
  filter_ = SkImageFilter::MakeMatrixFilter(
      ToSkMatrix(matrix4), static_cast<SkFilterQuality>(filterQuality),
      nullptr);
}

}  // namespace flutter
