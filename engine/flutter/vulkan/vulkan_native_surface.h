// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_VULKAN_VULKAN_NATIVE_SURFACE_H_
#define FLUTTER_VULKAN_VULKAN_NATIVE_SURFACE_H_

#ifndef RS_ENABLE_VK
#include "flutter/fml/macros.h"
#endif
#include "third_party/skia/include/core/SkSize.h"
#include "vulkan_handle.h"
#include "vulkan_proc_table.h"

namespace vulkan {

class VulkanNativeSurface {
 public:
  virtual ~VulkanNativeSurface() = default;

  virtual const char* GetExtensionName() const = 0;

  virtual uint32_t GetSkiaExtensionName() const = 0;

  virtual VkSurfaceKHR CreateSurfaceHandle(
      VulkanProcTable& vk,
      const VulkanHandle<VkInstance>& instance) const = 0;

  virtual bool IsValid() const = 0;

  virtual SkISize GetSize() const = 0;
};

}  // namespace vulkan

#endif  // FLUTTER_VULKAN_VULKAN_NATIVE_SURFACE_H_
