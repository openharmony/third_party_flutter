// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_VULKAN_VULKAN_WINDOW_H_
#define FLUTTER_VULKAN_VULKAN_WINDOW_H_

#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#ifndef RS_ENABLE_VK
#include "flutter/fml/compiler_specific.h"
#include "flutter/fml/macros.h"
#endif
#include "flutter/vulkan/vulkan_proc_table.h"
#include "third_party/skia/include/core/SkRefCnt.h"
#include "third_party/skia/include/core/SkSize.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/gpu/GrContext.h"
#include "third_party/skia/include/gpu/vk/GrVkBackendContext.h"

namespace vulkan {

class VulkanNativeSurface;
class VulkanDevice;
class VulkanSurface;
class VulkanSwapchain;
class VulkanImage;
class VulkanApplication;
class VulkanBackbuffer;

class VulkanWindow {
 public:
#ifdef RS_ENABLE_VK
  typedef std::shared_ptr<VulkanWindow> Ptr;
  VulkanWindow(std::unique_ptr<VulkanNativeSurface> native_surface,
               bool is_offscreen = false);
#else
  VulkanWindow(fml::RefPtr<VulkanProcTable> proc_table,
               std::unique_ptr<VulkanNativeSurface> native_surface);
#endif

  ~VulkanWindow();

  bool IsValid() const;

  GrContext* GetSkiaGrContext();

  sk_sp<SkSurface> AcquireSurface();

  bool SwapBuffers();

#ifdef RS_ENABLE_VK
  bool FlushCommands();
  static void PresentAll();
  static void InitializeVulkan(size_t thread_num = 0);
  static void WaitForSharedFence();
  static bool ResetSharedFence();
#endif

 private:
  bool valid_;
#ifdef RS_ENABLE_VK
  static VulkanProcTable* vk;
  static std::unique_ptr<VulkanApplication> application_;
  static std::unique_ptr<VulkanDevice> logical_device_;
  bool is_offscreen_ = false;
  static std::thread::id device_thread_;
  static std::vector<VulkanHandle<VkFence>> shared_fences_;
  static uint32_t shared_fence_index_;
  static bool presenting_;
#else
  fml::RefPtr<VulkanProcTable> vk;
  std::unique_ptr<VulkanApplication> application_;
  std::unique_ptr<VulkanDevice> logical_device_;
#endif
  std::unique_ptr<VulkanSurface> surface_;
  std::unique_ptr<VulkanSwapchain> swapchain_;
  sk_sp<GrContext> skia_gr_context_;

  bool CreateSkiaGrContext();

  bool CreateSkiaBackendContext(GrVkBackendContext* context);

#ifndef RS_ENABLE_VK
  FML_WARN_UNUSED_RESULT
#endif
  bool RecreateSwapchain();

#ifndef RS_ENABLE_VK
  FML_DISALLOW_COPY_AND_ASSIGN(VulkanWindow);
#endif
};

}  // namespace vulkan

#endif  // FLUTTER_VULKAN_VULKAN_WINDOW_H_
