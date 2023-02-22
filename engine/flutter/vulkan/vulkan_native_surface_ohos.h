// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_VULKAN_VULKAN_NATIVE_SURFACE_OHOS_H_
#define FLUTTER_VULKAN_VULKAN_NATIVE_SURFACE_OHOS_H_

#include "flutter/vulkan/vulkan_native_surface.h"
#include "window.h"

namespace vulkan {

class VulkanNativeSurfaceOHOS : public VulkanNativeSurface {
 public:
  /// Create a native surface from the valid NativeWindow reference. Ownership
  /// of the NativeWindow is assumed by this instance.
  VulkanNativeSurfaceOHOS(struct NativeWindow* native_window);

  ~VulkanNativeSurfaceOHOS();

  const char* GetExtensionName() const override;

  uint32_t GetSkiaExtensionName() const override;

  VkSurfaceKHR CreateSurfaceHandle(
      VulkanProcTable& vk,
      const VulkanHandle<VkInstance>& instance) const override;

  bool IsValid() const override;

  SkISize GetSize() const override;

 private:
  struct NativeWindow* native_window_;
};

}  // namespace vulkan

#endif  // FLUTTER_VULKAN_VULKAN_NATIVE_SURFACE_OHOS_H_
