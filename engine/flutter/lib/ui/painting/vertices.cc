// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/vertices.h"

#include <algorithm>

namespace flutter {

namespace {

void DecodePoints(const tonic::Float32List& coords, SkPoint* points) {
  for (size_t i = 0; i < coords.size(); i += 2)
    points[i / 2] = SkPoint::Make(coords[i], coords[i + 1]);
}

template <typename T>
void DecodeInts(const tonic::Int32List& ints, T* out) {
  for (size_t i = 0; i < ints.size(); i++)
    out[i] = ints[i];
}

}  // namespace

IMPLEMENT_WRAPPERTYPEINFO(ui, Vertices);

#define FOR_EACH_BINDING(V) V(Vertices, init)

FOR_EACH_BINDING(DART_NATIVE_CALLBACK)

Vertices::Vertices() {}

Vertices::~Vertices() {}

void Vertices::RegisterNatives(tonic::DartLibraryNatives* natives) {
}

fml::RefPtr<Vertices> Vertices::Create() {
  return fml::MakeRefCounted<Vertices>();
}

bool Vertices::init(SkVertices::VertexMode vertex_mode,
                    const tonic::Float32List& positions,
                    const tonic::Float32List& texture_coordinates,
                    const tonic::Int32List& colors,
                    const tonic::Uint16List& indices) {
  uint32_t builderFlags = 0;
  if (texture_coordinates.data())
    builderFlags |= SkVertices::kHasTexCoords_BuilderFlag;
  if (colors.data())
    builderFlags |= SkVertices::kHasColors_BuilderFlag;

  SkVertices::Builder builder(vertex_mode, positions.size() / 2,
                              indices.size(), builderFlags);

  if (!builder.isValid())
    return false;

  // positions are required for SkVertices::Builder
  FML_DCHECK(positions.data());
  if (positions.data())
    DecodePoints(positions, builder.positions());

  if (texture_coordinates.data()) {
    // SkVertices::Builder assumes equal numbers of elements
    FML_DCHECK(positions.size() == texture_coordinates.size());
    DecodePoints(texture_coordinates, builder.texCoords());
  }
  if (colors.data()) {
    // SkVertices::Builder assumes equal numbers of elements
    FML_DCHECK(positions.size() / 2 == colors.size());
    DecodeInts<SkColor>(colors, builder.colors());
  }

  if (indices.data()) {
    std::copy(indices.data(), indices.data() + indices.size(),
              builder.indices());
  }

  vertices_ = builder.detach();

  return true;
}

}  // namespace flutter
