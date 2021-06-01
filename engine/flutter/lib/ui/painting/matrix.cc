// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/matrix.h"

#include "flutter/fml/logging.h"

namespace flutter {

// Mappings from SkMatrix-index to input-index.
static const size_t kSkMatrixIndexToMatrix4Index[] = {
    // clang-format off
    0, 4, 12,
    1, 5, 13,
    3, 7, 15,
    // clang-format on
};

SkMatrix ToSkMatrix(const tonic::Float64List& matrix4) {
  FML_DCHECK(!matrix4.empty());
  SkMatrix sk_matrix;
  for (int i = 0; i < 9; ++i) {
    size_t matrix4_index = kSkMatrixIndexToMatrix4Index[i];
    if (matrix4_index < matrix4.size())
      sk_matrix[i] = matrix4[matrix4_index];
    else
      sk_matrix[i] = 0.0;
  }
  return sk_matrix;
}

}  // namespace flutter
