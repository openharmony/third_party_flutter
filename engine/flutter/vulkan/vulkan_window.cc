// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/vulkan/vulkan_window.h"

#include <memory>
#include <string>

#include "flutter/vulkan/vulkan_application.h"
#include "flutter/vulkan/vulkan_device.h"
#ifdef RS_ENABLE_VK
#include "flutter/vulkan/vulkan_hilog.h"
#endif
#include "flutter/vulkan/vulkan_native_surface.h"
#include "flutter/vulkan/vulkan_surface.h"
#include "flutter/vulkan/vulkan_swapchain.h"
#include "third_party/skia/include/gpu/GrContext.h"

namespace vulkan {

#ifdef RS_ENABLE_VK
VulkanProcTable* VulkanWindow::vk;
std::unique_ptr<VulkanApplication> VulkanWindow::application_;
std::unique_ptr<VulkanDevice> VulkanWindow::logical_device_;
std::thread::id VulkanWindow::device_thread_;
std::vector<VulkanHandle<VkFence>> VulkanWindow::shared_fences_;
uint32_t VulkanWindow::shared_fence_index_;
bool VulkanWindow::presenting_ = false;

void VulkanWindow::InitializeVulkan(size_t thread_num)
{
  if (shared_fences_.size() < thread_num) {
    shared_fences_.resize(thread_num);
    shared_fence_index_ = 0;
  }

  if (logical_device_ != nullptr) {
    LOGI("Vulkan Already Initialized");
    return;
  }

  LOGI("First Initialize Vulkan");
  device_thread_ = std::this_thread::get_id();

  vk = new VulkanProcTable();
  if (!vk->HasAcquiredMandatoryProcAddresses()) {
    LOGE("Proc table has not acquired mandatory proc addresses.");
    return;
  }

  // Create the application instance.
  std::vector<std::string> extensions = {
      VK_KHR_SURFACE_SPEC_VERSION,               // parent extension
      VK_OPENHARMONY_OHOS_SURFACE_EXTENSION_NAME // child extension
  };

  application_ = std::make_unique<VulkanApplication>(*vk, "Rosen", std::move(extensions));
  if (!application_->IsValid() || !vk->AreInstanceProcsSetup()) {
    // Make certain the application instance was created and it setup the
    // instance proc table entries.
    LOGE("Instance proc addresses have not been setup.");
    return;
  }

  // Create the device.
  logical_device_ = application_->AcquireFirstCompatibleLogicalDevice();
  if (logical_device_ == nullptr || !logical_device_->IsValid() || !vk->AreDeviceProcsSetup()) {
    // Make certain the device was created and it setup the device proc table
    // entries.
    LOGE("Device proc addresses have not been setup.");
    return;
  }
}

VulkanWindow::VulkanWindow(std::unique_ptr<VulkanNativeSurface> native_surface, bool is_offscreen)
    : valid_(false), is_offscreen_(is_offscreen)
{
  LOGE("VulkanWindow init enter");

  InitializeVulkan();
  if (logical_device_ == nullptr) {
    LOGE("InitializeVulkan failed");
    return;
  }

  if (!is_offscreen && (native_surface == nullptr || !native_surface->IsValid())) {
    LOGE("Native surface is invalid.");
    return;
  }

  // Create the logical surface from the native platform surface.
  if (!is_offscreen) {
    surface_ = std::make_unique<VulkanSurface>(*vk, *application_, std::move(native_surface));
    if (!surface_->IsValid()) {
      LOGE("Vulkan surface is invalid.");
      return;
    }
  }

  // Create the Skia GrContext.
  if (!CreateSkiaGrContext()) {
    LOGE("Could not create Skia context.");
    return;
  }

  // Create the swapchain.
  if (!is_offscreen && !RecreateSwapchain()) {
    LOGE("Could not setup the swapchain initially.");
    return;
  }
  LOGE("VulkanWindow init success");
  valid_ = true;
}
#else
VulkanWindow::VulkanWindow(fml::RefPtr<VulkanProcTable> proc_table,
                           std::unique_ptr<VulkanNativeSurface> native_surface)
    : valid_(false), vk(std::move(proc_table)) {
  if (!vk || !vk->HasAcquiredMandatoryProcAddresses()) {
    FML_DLOG(INFO) << "Proc table has not acquired mandatory proc addresses.";
    return;
  }

  if (native_surface == nullptr || !native_surface->IsValid()) {
    FML_DLOG(INFO) << "Native surface is invalid.";
    return;
  }

  // Create the application instance.

  std::vector<std::string> extensions = {
      VK_KHR_SURFACE_EXTENSION_NAME,      // parent extension
      native_surface->GetExtensionName()  // child extension
  };

  application_ = std::make_unique<VulkanApplication>(*vk, "Flutter",
                                                     std::move(extensions));

  if (!application_->IsValid() || !vk->AreInstanceProcsSetup()) {
    // Make certain the application instance was created and it setup the
    // instance proc table entries.
    FML_DLOG(INFO) << "Instance proc addresses have not been setup.";
    return;
  }

  // Create the device.

  logical_device_ = application_->AcquireFirstCompatibleLogicalDevice();

  if (logical_device_ == nullptr || !logical_device_->IsValid() ||
      !vk->AreDeviceProcsSetup()) {
    // Make certain the device was created and it setup the device proc table
    // entries.
    FML_DLOG(INFO) << "Device proc addresses have not been setup.";
    return;
  }

  // Create the logical surface from the native platform surface.

  surface_ = std::make_unique<VulkanSurface>(*vk, *application_,
                                             std::move(native_surface));

  if (!surface_->IsValid()) {
    FML_DLOG(INFO) << "Vulkan surface is invalid.";
    return;
  }

  // Create the Skia GrContext.

  if (!CreateSkiaGrContext()) {
    FML_DLOG(INFO) << "Could not create Skia context.";
    return;
  }

  // Create the swapchain.

  if (!RecreateSwapchain()) {
    FML_DLOG(INFO) << "Could not setup the swapchain initially.";
    return;
  }

  valid_ = true;
}
#endif

VulkanWindow::~VulkanWindow() = default;

bool VulkanWindow::IsValid() const {
  return valid_;
}

GrContext* VulkanWindow::GetSkiaGrContext() {
  return skia_gr_context_.get();
}

bool VulkanWindow::CreateSkiaGrContext() {
  GrVkBackendContext backend_context;

  if (!CreateSkiaBackendContext(&backend_context)) {
#ifdef RS_ENABLE_VK
    LOGE("CreateSkiaGrContext CreateSkiaBackendContext is false");
#endif
    return false;
  }

  sk_sp<GrContext> context = GrContext::MakeVulkan(backend_context);

  if (context == nullptr) {
#ifdef RS_ENABLE_VK
    LOGE("CreateSkiaGrContext context is null");
#endif
    return false;
  }

  context->setResourceCacheLimits(kGrCacheMaxCount, kGrCacheMaxByteSize);

  skia_gr_context_ = context;

  return true;
}

bool VulkanWindow::CreateSkiaBackendContext(GrVkBackendContext* context) {
  auto getProc = vk->CreateSkiaGetProc();

  if (getProc == nullptr) {
#ifdef RS_ENABLE_VK
    LOGE("CreateSkiaBackendContext getProc is null");
#endif
    return false;
  }

  uint32_t skia_features = 0;
  if (!logical_device_->GetPhysicalDeviceFeaturesSkia(&skia_features)) {
#ifdef RS_ENABLE_VK
    LOGE("CreateSkiaBackendContext GetPhysicalDeviceFeaturesSkia is false");
#endif
    return false;
  }

  context->fInstance = application_->GetInstance();
  context->fPhysicalDevice = logical_device_->GetPhysicalDeviceHandle();
  context->fDevice = logical_device_->GetHandle();
  context->fQueue = logical_device_->GetQueueHandle();
  context->fGraphicsQueueIndex = logical_device_->GetGraphicsQueueIndex();
  context->fMinAPIVersion = application_->GetAPIVersion();
#ifdef RS_ENABLE_VK
  uint32_t extensionFlags = kKHR_surface_GrVkExtensionFlag;
  if (!is_offscreen_) {
    extensionFlags |= kKHR_swapchain_GrVkExtensionFlag;
    extensionFlags |= surface_->GetNativeSurface().GetSkiaExtensionName();
  }
  context->fExtensions = extensionFlags;
#else
  context->fExtensions = kKHR_surface_GrVkExtensionFlag |
                         kKHR_swapchain_GrVkExtensionFlag |
                         surface_->GetNativeSurface().GetSkiaExtensionName();
#endif
  context->fFeatures = skia_features;
  context->fGetProc = std::move(getProc);
  context->fOwnsInstanceAndDevice = false;
  return true;
}

sk_sp<SkSurface> VulkanWindow::AcquireSurface() {
#ifdef RS_ENABLE_VK
  if (is_offscreen_ || !IsValid()) {
    LOGE("Surface is invalid or offscreen.");
    return nullptr;
  }
#else
  if (!IsValid()) {
    FML_DLOG(INFO) << "Surface is invalid.";
    return nullptr;
  }
#endif

  auto surface_size = surface_->GetSize();

  // This check is theoretically unnecessary as the swapchain should report that
  // the surface is out-of-date and perform swapchain recreation at the new
  // configuration. However, on Android, the swapchain never reports that it is
  // of date. Hence this extra check. Platforms that don't have this issue, or,
  // cant report this information (which is optional anyway), report a zero
  // size.
  if (surface_size != SkISize::Make(0, 0) &&
      surface_size != swapchain_->GetSize()) {
#ifdef RS_ENABLE_VK
    LOGE("Swapchain and surface sizes are out of sync. Recreating swapchain.");
#else
    FML_DLOG(INFO) << "Swapchain and surface sizes are out of sync. Recreating "
                      "swapchain.";
#endif
    if (!RecreateSwapchain()) {
#ifdef RS_ENABLE_VK
      LOGE("Could not recreate swapchain.");
#else
      FML_DLOG(INFO) << "Could not recreate swapchain.";
#endif
      valid_ = false;
      return nullptr;
    }
  }

  while (true) {
    sk_sp<SkSurface> surface;
    auto acquire_result = VulkanSwapchain::AcquireStatus::ErrorSurfaceLost;

    std::tie(acquire_result, surface) = swapchain_->AcquireSurface();

    if (acquire_result == VulkanSwapchain::AcquireStatus::Success) {
      // Successfully acquired a surface from the swapchain. Nothing more to do.
      return surface;
    }

    if (acquire_result == VulkanSwapchain::AcquireStatus::ErrorSurfaceLost) {
      // Surface is lost. This is an unrecoverable error.
#ifdef RS_ENABLE_VK
      LOGE("Swapchain reported surface was lost.");
#else
      FML_DLOG(INFO) << "Swapchain reported surface was lost.";
#endif
      return nullptr;
    }

    if (acquire_result ==
        VulkanSwapchain::AcquireStatus::ErrorSurfaceOutOfDate) {
#ifdef RS_ENABLE_VK
      LOGE("AcquireSurface surface out of date");
#endif
      // Surface out of date. Recreate the swapchain at the new configuration.
      if (RecreateSwapchain()) {
        // Swapchain was recreated, try surface acquisition again.
        continue;
      } else {
        // Could not recreate the swapchain at the new configuration.
#ifdef RS_ENABLE_VK
        LOGE("Swapchain reported surface was out of date but "
                           "could not recreate the swapchain at the new "
                           "configuration.");
#else
        FML_DLOG(INFO) << "Swapchain reported surface was out of date but "
                          "could not recreate the swapchain at the new "
                          "configuration.";
#endif
        valid_ = false;
        return nullptr;
      }
    }

    break;
  }

#ifdef RS_ENABLE_VK
  LOGE("Unhandled VulkanSwapchain::AcquireResult");
#else
  FML_DCHECK(false) << "Unhandled VulkanSwapchain::AcquireResult";
#endif
  return nullptr;
}

bool VulkanWindow::SwapBuffers() {
#ifdef RS_ENABLE_VK
  if (is_offscreen_ || !IsValid()) {
      LOGE("Window was invalid or offscreen.");
      return false;
  }
  if (device_thread_ != std::this_thread::get_id()) {
    LOGI("MT mode in VulkanWindow::SwapBuffers()");
    swapchain_->AddToPresent();
    return swapchain_->FlushCommands();
  }
  LOGI("ST mode in VulkanWindow::SwapBuffers()");
#else
  if (!IsValid()) {
    FML_DLOG(INFO) << "Window was invalid.";
    return false;
  }
#endif
  return swapchain_->Submit();
}

#ifdef RS_ENABLE_VK
void VulkanWindow::PresentAll() {
  //-----------------------------------------
  // create shared fences if not already
  //-----------------------------------------
  if (!shared_fences_[shared_fence_index_]) {
    const VkFenceCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    auto fence_collect = [](VkFence fence) {
      VulkanWindow::vk->DestroyFence(VulkanWindow::logical_device_->GetHandle(), fence, nullptr);
    };

    VkFence fence = VK_NULL_HANDLE;

    if (VK_CALL_LOG_ERROR(vk->CreateFence(logical_device_->GetHandle(), &create_info, nullptr, &fence)) != VK_SUCCESS) {
      return;
    }
    shared_fences_[shared_fence_index_] = {fence, fence_collect};
  }
  VulkanSwapchain::PresentAll(shared_fences_[shared_fence_index_]);
  shared_fence_index_++;
  if (shared_fence_index_ >= shared_fences_.size()) {
    shared_fence_index_ = 0;
  }
  presenting_ = true;
}

bool VulkanWindow::WaitForSharedFence() {
  if (presenting_) {
    if (shared_fences_[shared_fence_index_]) {
      VkFence fence = shared_fences_[shared_fence_index_];
      return VK_CALL_LOG_ERROR(vk->WaitForFences(
        logical_device_->GetHandle(), 1, &fence, true,
        std::numeric_limits<uint64_t>::max())) == VK_SUCCESS;
    }
  }
  return false;
}

bool VulkanWindow::ResetSharedFence() {
  if (presenting_) {
      presenting_ = false;
      if (shared_fences_[shared_fence_index_]) {
        VkFence fence = shared_fences_[shared_fence_index_];
        return VK_CALL_LOG_ERROR(vk->ResetFences(
            logical_device_->GetHandle(), 1, &fence)) == VK_SUCCESS;
      }
  }
  return false;
}
#endif

bool VulkanWindow::RecreateSwapchain() {
#ifdef RS_ENABLE_VK
  if (is_offscreen_) {
      LOGE("offscreen vulkan window, don't need swapchian");
      return false;
  }
#endif
  // This way, we always lose our reference to the old swapchain. Even if we
  // cannot create a new one to replace it.
  auto old_swapchain = std::move(swapchain_);

  if (!vk->IsValid()) {
#ifdef RS_ENABLE_VK
    LOGE("RecreateSwapchain vk not valid");
#endif
    return false;
  }

  if (logical_device_ == nullptr || !logical_device_->IsValid()) {
#ifdef RS_ENABLE_VK
    LOGE("RecreateSwapchain logical_device_ not valid");
#endif
    return false;
  }

  if (surface_ == nullptr || !surface_->IsValid()) {
#ifdef RS_ENABLE_VK
    LOGE("RecreateSwapchain surface_ not valid");
#endif
    return false;
  }

  if (skia_gr_context_ == nullptr) {
#ifdef RS_ENABLE_VK
    LOGE("RecreateSwapchain skia_gr_context_ not valid");
#endif
    return false;
  }

  auto swapchain = std::make_unique<VulkanSwapchain>(
      *vk, *logical_device_, *surface_, skia_gr_context_.get(),
      std::move(old_swapchain), logical_device_->GetGraphicsQueueIndex());

  if (!swapchain->IsValid()) {
#ifdef RS_ENABLE_VK
    LOGE("RecreateSwapchain swapchain not valid");
#endif
    return false;
  }

  swapchain_ = std::move(swapchain);
  return true;
}

}  // namespace vulkan
