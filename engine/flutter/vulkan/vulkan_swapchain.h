// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_VULKAN_VULKAN_SWAPCHAIN_H_
#define FLUTTER_VULKAN_VULKAN_SWAPCHAIN_H_

#include <memory>
#include <utility>
#include <vector>
#include <thread>
#include <mutex>
#include <unordered_map>

#ifndef RS_ENABLE_VK
#include "flutter/fml/compiler_specific.h"
#include "flutter/fml/macros.h"
#endif
#include "flutter/vulkan/vulkan_handle.h"
#include "third_party/skia/include/core/SkSize.h"
#include "third_party/skia/include/core/SkSurface.h"

namespace vulkan {

class VulkanProcTable;
class VulkanDevice;
class VulkanSurface;
class VulkanBackbuffer;
class VulkanImage;

class VulkanSwapchain {
 public:
  VulkanSwapchain(const VulkanProcTable& vk,
                  const VulkanDevice& device,
                  const VulkanSurface& surface,
                  GrContext* skia_context,
                  std::unique_ptr<VulkanSwapchain> old_swapchain,
                  uint32_t queue_family_index);

  ~VulkanSwapchain();

  bool IsValid() const;

  enum class AcquireStatus {
    /// A valid SkSurface was acquired successfully from the swapchain.
    Success,
    /// The underlying surface of the swapchain was permanently lost. This is an
    /// unrecoverable error. The entire surface must be recreated along with the
    /// swapchain.
    ErrorSurfaceLost,
    /// The swapchain surface is out-of-date with the underlying surface. The
    /// swapchain must be recreated.
    ErrorSurfaceOutOfDate,
  };
  using AcquireResult = std::pair<AcquireStatus, sk_sp<SkSurface>>;

  /// Acquire an SkSurface from the swapchain for the caller to render into for
  /// later submission via |Submit|. There must not be consecutive calls to
  /// |AcquireFrame| without and interleaving |Submit|.
  AcquireResult AcquireSurface();

  /// Submit a previously acquired. There must not be consecutive calls to
  /// |Submit| without and interleaving |AcquireFrame|.
#ifndef RS_ENABLE_VK
  FML_WARN_UNUSED_RESULT
#endif
  bool Submit();

  SkISize GetSize() const;

#ifdef RS_ENABLE_VK
  bool FlushCommands();

  void AddToPresent();

  static void PresentAll(VulkanHandle<VkFence>& shared_fence);

 private:
  const VulkanProcTable& vk;
  const VulkanDevice& device_;
  VkSurfaceCapabilitiesKHR capabilities_;
  VkSurfaceFormatKHR surface_format_;
  VulkanHandle<VkSwapchainKHR> swapchain_;
  std::vector<std::unique_ptr<VulkanBackbuffer>> backbuffers_;
  std::vector<std::unique_ptr<VulkanImage>> images_;
  std::vector<sk_sp<SkSurface>> surfaces_;
  VkPipelineStageFlagBits current_pipeline_stage_;
  size_t current_backbuffer_index_;
  size_t current_image_index_;
  bool valid_;
  static std::mutex map_mutex_;
  static std::unordered_map<std::thread::id, VulkanSwapchain*> to_be_present_;

  std::vector<VkImage> GetImages() const;

  bool CreateSwapchainImages(GrContext* skia_context,
                             SkColorType color_type,
                             sk_sp<SkColorSpace> color_space);

  sk_sp<SkSurface> CreateSkiaSurface(GrContext* skia_context,
                                     VkImage image,
                                     const SkISize& size,
                                     SkColorType color_type,
                                     sk_sp<SkColorSpace> color_space) const;

  VulkanBackbuffer* GetNextBackbuffer();
#else
#if OS_ANDROID
 private:
  const VulkanProcTable& vk;
  const VulkanDevice& device_;
  VkSurfaceCapabilitiesKHR capabilities_;
  VkSurfaceFormatKHR surface_format_;
  VulkanHandle<VkSwapchainKHR> swapchain_;
  std::vector<std::unique_ptr<VulkanBackbuffer>> backbuffers_;
  std::vector<std::unique_ptr<VulkanImage>> images_;
  std::vector<sk_sp<SkSurface>> surfaces_;
  VkPipelineStageFlagBits current_pipeline_stage_;
  size_t current_backbuffer_index_;
  size_t current_image_index_;
  bool valid_;

  std::vector<VkImage> GetImages() const;

  bool CreateSwapchainImages(GrContext* skia_context,
                             SkColorType color_type,
                             sk_sp<SkColorSpace> color_space);

  sk_sp<SkSurface> CreateSkiaSurface(GrContext* skia_context,
                                     VkImage image,
                                     const SkISize& size,
                                     SkColorType color_type,
                                     sk_sp<SkColorSpace> color_space) const;

  VulkanBackbuffer* GetNextBackbuffer();
#endif  // OS_ANDROID
#endif  // RS_ENABLE_VK

#ifndef RS_ENABLE_VK
  FML_DISALLOW_COPY_AND_ASSIGN(VulkanSwapchain);
#endif
};

}  // namespace vulkan

#endif  // FLUTTER_VULKAN_VULKAN_SWAPCHAIN_H_
