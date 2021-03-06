// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_GPU_GPU_SURFACE_GL_DELEGATE_H_
#define FLUTTER_SHELL_GPU_GPU_SURFACE_GL_DELEGATE_H_

#include "flutter/flow/embedded_views.h"
#include "flutter/fml/macros.h"
#include "third_party/skia/include/core/SkMatrix.h"
#include "third_party/skia/include/gpu/gl/GrGLInterface.h"

namespace flutter {

class GPUSurfaceGLDelegate {
 public:
  // Called to make the main GL context current on the current thread.
  virtual bool GLContextMakeCurrent() = 0;

  // Called to clear the current GL context on the thread. This may be called on
  // either the GPU or IO threads.
  virtual bool GLContextClearCurrent() = 0;

  // Called to present the main GL surface. This is only called for the main GL
  // context and not any of the contexts dedicated for IO.
  virtual bool GLContextPresent() = 0;

  // For PC preivew
  virtual bool GLContextSendSurface(const void* pixels,
                                    const size_t size,
                                    const int32_t width,
                                    const int32_t height) {
    return false;
  };

  // The ID of the main window bound framebuffer. Typically FBO0.
  virtual intptr_t GLContextFBO() const = 0;

  // The rendering subsystem assumes that the ID of the main window bound
  // framebuffer remains constant throughout. If this assumption in incorrect,
  // embedders are required to return true from this method. In such cases,
  // GLContextFBO() will be called again to acquire the new FBO ID for rendering
  // subsequent frames.
  virtual bool GLContextFBOResetAfterPresent() const;

  // Create an offscreen surface to render into before onscreen composition.
  virtual bool UseOffscreenSurface() const;

  // A transformation applied to the onscreen surface before the canvas is
  // flushed.
  virtual SkMatrix GLContextSurfaceTransformation() const;

  // Get a reference to the external views embedder. This happens on the same
  // thread that the renderer is operating on.
  virtual ExternalViewEmbedder* GetExternalViewEmbedder() = 0;

  sk_sp<const GrGLInterface> GetGLInterface() const;

  // TODO(chinmaygarde): The presence of this method is to work around the fact
  // that not all platforms can accept a custom GL proc table. Migrate all
  // platforms to move GL proc resolution to the embedder and remove this
  // method.
  static sk_sp<const GrGLInterface> GetDefaultPlatformGLInterface();

  using GLProcResolver =
      std::function<void* /* proc name */ (const char* /* proc address */)>;
  // Provide a custom GL proc resolver. If no such resolver is present, Skia
  // will attempt to do GL proc address resolution on its own. Embedders that
  // have specific opinions on GL API selection or need to add their own
  // instrumentation to specific GL calls can specify custom GL functions
  // here.
  virtual GLProcResolver GetGLProcResolver() const;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_GPU_GPU_SURFACE_GL_DELEGATE_H_
