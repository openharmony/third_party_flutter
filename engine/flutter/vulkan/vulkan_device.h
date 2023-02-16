// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_VULKAN_VULKAN_DEVICE_H_
#define FLUTTER_VULKAN_VULKAN_DEVICE_H_

#include <vector>

#ifndef RS_ENABLE_VK
#include "flutter/fml/compiler_specific.h"
#include "flutter/fml/macros.h"
#endif
#include "flutter/vulkan/vulkan_handle.h"

namespace vulkan {

class VulkanProcTable;
class VulkanSurface;

class VulkanDevice {
 public:
  VulkanDevice(VulkanProcTable& vk,
               VulkanHandle<VkPhysicalDevice> physical_device);

  ~VulkanDevice();

  bool IsValid() const;

  const VulkanHandle<VkDevice>& GetHandle() const;

  const VulkanHandle<VkPhysicalDevice>& GetPhysicalDeviceHandle() const;

  const VulkanHandle<VkQueue>& GetQueueHandle() const;

  const VulkanHandle<VkCommandPool>& GetCommandPool() const;

  uint32_t GetGraphicsQueueIndex() const;

  void ReleaseDeviceOwnership();

#ifndef RS_ENABLE_VK
  FML_WARN_UNUSED_RESULT
#endif
  bool GetSurfaceCapabilities(const VulkanSurface& surface,
                              VkSurfaceCapabilitiesKHR* capabilities) const;

#ifndef RS_ENABLE_VK
  FML_WARN_UNUSED_RESULT
#endif
  bool GetPhysicalDeviceFeatures(VkPhysicalDeviceFeatures* features) const;

#ifndef RS_ENABLE_VK
  FML_WARN_UNUSED_RESULT
#endif
  bool GetPhysicalDeviceFeaturesSkia(
      uint32_t* /* mask of GrVkFeatureFlags */ features) const;

#ifndef RS_ENABLE_VK
  FML_WARN_UNUSED_RESULT
#endif
  int ChooseSurfaceFormat(const VulkanSurface& surface,
                          std::vector<VkFormat> desired_formats,
                          VkSurfaceFormatKHR* format) const;

#ifndef RS_ENABLE_VK
  FML_WARN_UNUSED_RESULT
#endif
  bool ChoosePresentMode(const VulkanSurface& surface,
                         VkPresentModeKHR* present_mode) const;

#ifndef RS_ENABLE_VK
  FML_WARN_UNUSED_RESULT
#endif
  bool QueueSubmit(std::vector<VkPipelineStageFlags> wait_dest_pipeline_stages,
                   const std::vector<VkSemaphore>& wait_semaphores,
                   const std::vector<VkSemaphore>& signal_semaphores,
                   const std::vector<VkCommandBuffer>& command_buffers,
                   const VulkanHandle<VkFence>& fence) const;

#ifndef RS_ENABLE_VK
  FML_WARN_UNUSED_RESULT
#endif
  bool WaitIdle() const;

 private:
  VulkanProcTable& vk;
  VulkanHandle<VkPhysicalDevice> physical_device_;
  VulkanHandle<VkDevice> device_;
  VulkanHandle<VkQueue> queue_;
  VulkanHandle<VkCommandPool> command_pool_;
  uint32_t graphics_queue_index_;
#ifdef RS_ENABLE_VK
  uint32_t compute_queue_index_;
#endif
  bool valid_;

  std::vector<VkQueueFamilyProperties> GetQueueFamilyProperties() const;

#ifndef RS_ENABLE_VK
  FML_DISALLOW_COPY_AND_ASSIGN(VulkanDevice);
#endif
};

}  // namespace vulkan

#endif  // FLUTTER_VULKAN_VULKAN_DEVICE_H_
