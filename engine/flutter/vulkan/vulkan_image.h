// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_VULKAN_VULKAN_IMAGE_H_
#define FLUTTER_VULKAN_VULKAN_IMAGE_H_

#ifndef RS_ENABLE_VK
#include "flutter/fml/compiler_specific.h"
#include "flutter/fml/macros.h"
#endif
#include "flutter/vulkan/vulkan_handle.h"

namespace vulkan {

class VulkanProcTable;
class VulkanCommandBuffer;

class VulkanImage {
 public:
  VulkanImage(VulkanHandle<VkImage> image);

  ~VulkanImage();

  bool IsValid() const;

#ifndef RS_ENABLE_VK
  FML_WARN_UNUSED_RESULT
#endif
  bool InsertImageMemoryBarrier(const VulkanCommandBuffer& command_buffer,
                                VkPipelineStageFlagBits src_pipline_bits,
                                VkPipelineStageFlagBits dest_pipline_bits,
                                VkAccessFlagBits dest_access_flags,
                                VkImageLayout dest_layout);

 private:
  VulkanHandle<VkImage> handle_;
  VkImageLayout layout_;
  uint32_t /* mask of VkAccessFlagBits */ access_flags_;
  bool valid_;

#ifndef RS_ENABLE_VK
  FML_DISALLOW_COPY_AND_ASSIGN(VulkanImage);
#endif
};

}  // namespace vulkan

#endif  // FLUTTER_VULKAN_VULKAN_IMAGE_H_
