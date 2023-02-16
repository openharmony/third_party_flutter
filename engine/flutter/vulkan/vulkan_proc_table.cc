// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/vulkan/vulkan_proc_table.h"

#include <dlfcn.h>

#ifndef RS_ENABLE_VK
#include "flutter/fml/logging.h"
#endif
#ifdef RS_ENABLE_VK
#include "flutter/vulkan/vulkan_hilog.h"
#endif

#ifdef RS_ENABLE_VK
#define ACQUIRE_PROC(name, context)                          \
  if (!(name = AcquireProc("vk" #name, context))) {          \
    LOGE("Could not acquire proc: vk" #name);                \
    return false;                                            \
  }
#else
#define ACQUIRE_PROC(name, context)                          \
  if (!(name = AcquireProc("vk" #name, context))) {          \
    FML_DLOG(INFO) << "Could not acquire proc: vk" << #name; \
    return false;                                            \
  }
#endif

namespace vulkan {

VulkanProcTable::VulkanProcTable()
    : handle_(nullptr), acquired_mandatory_proc_addresses_(false) {
  acquired_mandatory_proc_addresses_ =
      OpenLibraryHandle() && SetupLoaderProcAddresses();
}

VulkanProcTable::~VulkanProcTable() {
  CloseLibraryHandle();
}

bool VulkanProcTable::HasAcquiredMandatoryProcAddresses() const {
  return acquired_mandatory_proc_addresses_;
}

bool VulkanProcTable::IsValid() const {
  return instance_ && device_;
}

bool VulkanProcTable::AreInstanceProcsSetup() const {
  return instance_;
}

bool VulkanProcTable::AreDeviceProcsSetup() const {
  return device_;
}

bool VulkanProcTable::SetupLoaderProcAddresses() {
  if (handle_ == nullptr) {
    return true;
  }

#ifdef RS_ENABLE_VK
  GetInstanceProcAddr =
#if VULKAN_LINK_STATICALLY
      GetInstanceProcAddr = &vkGetInstanceProcAddr;
#else  // VULKAN_LINK_STATICALLY
      reinterpret_cast<PFN_vkGetInstanceProcAddr>(dlsym(handle_, "vkGetInstanceProcAddr"));
      GetDeviceProcAddr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(dlsym(handle_, "vkGetDeviceProcAddr"));
      EnumerateInstanceExtensionProperties = reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(
          dlsym(handle_, "vkEnumerateInstanceExtensionProperties"));
      CreateInstance = reinterpret_cast<PFN_vkCreateInstance>(dlsym(handle_, "vkCreateInstance"));
#endif // VULKAN_LINK_STATICALLY
#else
  GetInstanceProcAddr =
#if VULKAN_LINK_STATICALLY
      GetInstanceProcAddr = &vkGetInstanceProcAddr;
#else   // VULKAN_LINK_STATICALLY
      reinterpret_cast<PFN_vkGetInstanceProcAddr>(
          dlsym(handle_, "vkGetInstanceProcAddr"));
#endif  // VULKAN_LINK_STATICALLY
#endif

  if (!GetInstanceProcAddr) {
#ifdef RS_ENABLE_VK
    LOGE("Could not acquire vkGetInstanceProcAddr.");
#else
    FML_DLOG(WARNING) << "Could not acquire vkGetInstanceProcAddr.";
#endif
    return false;
  }

  VulkanHandle<VkInstance> null_instance(VK_NULL_HANDLE, nullptr);

#ifndef RS_ENABLE_VK
  ACQUIRE_PROC(CreateInstance, null_instance);
  ACQUIRE_PROC(EnumerateInstanceExtensionProperties, null_instance);
#endif
  ACQUIRE_PROC(EnumerateInstanceLayerProperties, null_instance);

  return true;
}

bool VulkanProcTable::SetupInstanceProcAddresses(
    const VulkanHandle<VkInstance>& handle) {
  ACQUIRE_PROC(CreateDevice, handle);
  ACQUIRE_PROC(DestroyDevice, handle);
  ACQUIRE_PROC(DestroyInstance, handle);
  ACQUIRE_PROC(EnumerateDeviceLayerProperties, handle);
  ACQUIRE_PROC(EnumeratePhysicalDevices, handle);
#ifdef RS_ENABLE_VK
  ACQUIRE_PROC(GetPhysicalDeviceFeatures, handle);
  ACQUIRE_PROC(GetPhysicalDeviceQueueFamilyProperties, handle);
  ACQUIRE_PROC(GetPhysicalDeviceSurfaceCapabilitiesKHR, handle);
  ACQUIRE_PROC(GetPhysicalDeviceSurfaceFormatsKHR, handle);
  ACQUIRE_PROC(GetPhysicalDeviceSurfacePresentModesKHR, handle);
  ACQUIRE_PROC(GetPhysicalDeviceSurfaceSupportKHR, handle);
  ACQUIRE_PROC(DestroySurfaceKHR, handle);
  ACQUIRE_PROC(CreateOHOSSurfaceOpenHarmony, handle);
#else
  ACQUIRE_PROC(GetDeviceProcAddr, handle);
  ACQUIRE_PROC(GetPhysicalDeviceFeatures, handle);
  ACQUIRE_PROC(GetPhysicalDeviceQueueFamilyProperties, handle);
#if OS_ANDROID
  ACQUIRE_PROC(GetPhysicalDeviceSurfaceCapabilitiesKHR, handle);
  ACQUIRE_PROC(GetPhysicalDeviceSurfaceFormatsKHR, handle);
  ACQUIRE_PROC(GetPhysicalDeviceSurfacePresentModesKHR, handle);
  ACQUIRE_PROC(GetPhysicalDeviceSurfaceSupportKHR, handle);
  ACQUIRE_PROC(DestroySurfaceKHR, handle);
  ACQUIRE_PROC(CreateAndroidSurfaceKHR, handle);
#endif  // OS_ANDROID
#endif // RS_ENABLE_VK

  // The debug report functions are optional. We don't want proc acquisition to
  // fail here because the optional methods were not present (since ACQUIRE_PROC
  // returns false on failure). Wrap the optional proc acquisitions in an
  // anonymous lambda and invoke it. We don't really care about the result since
  // users of Debug reporting functions check for their presence explicitly.
#ifndef RS_ENABLE_VK
  [this, &handle]() -> bool {
    ACQUIRE_PROC(CreateDebugReportCallbackEXT, handle);
    ACQUIRE_PROC(DestroyDebugReportCallbackEXT, handle);
    return true;
  }();
#endif

  instance_ = {handle, nullptr};
  return true;
}

bool VulkanProcTable::SetupDeviceProcAddresses(
    const VulkanHandle<VkDevice>& handle) {
  ACQUIRE_PROC(AllocateCommandBuffers, handle);
  ACQUIRE_PROC(AllocateMemory, handle);
  ACQUIRE_PROC(BeginCommandBuffer, handle);
  ACQUIRE_PROC(BindImageMemory, handle);
  ACQUIRE_PROC(CmdPipelineBarrier, handle);
  ACQUIRE_PROC(CreateCommandPool, handle);
  ACQUIRE_PROC(CreateFence, handle);
  ACQUIRE_PROC(CreateImage, handle);
  ACQUIRE_PROC(CreateSemaphore, handle);
  ACQUIRE_PROC(DestroyCommandPool, handle);
  ACQUIRE_PROC(DestroyFence, handle);
  ACQUIRE_PROC(DestroyImage, handle);
  ACQUIRE_PROC(DestroySemaphore, handle);
  ACQUIRE_PROC(DeviceWaitIdle, handle);
  ACQUIRE_PROC(EndCommandBuffer, handle);
  ACQUIRE_PROC(FreeCommandBuffers, handle);
  ACQUIRE_PROC(FreeMemory, handle);
  ACQUIRE_PROC(GetDeviceQueue, handle);
  ACQUIRE_PROC(GetImageMemoryRequirements, handle);
  ACQUIRE_PROC(QueueSubmit, handle);
  ACQUIRE_PROC(QueueWaitIdle, handle);
  ACQUIRE_PROC(ResetCommandBuffer, handle);
  ACQUIRE_PROC(ResetFences, handle);
  ACQUIRE_PROC(WaitForFences, handle);
#ifdef RS_ENABLE_VK
  ACQUIRE_PROC(AcquireNextImageKHR, handle);
  ACQUIRE_PROC(CreateSwapchainKHR, handle);
  ACQUIRE_PROC(DestroySwapchainKHR, handle);
  ACQUIRE_PROC(GetSwapchainImagesKHR, handle);
  ACQUIRE_PROC(QueuePresentKHR, handle);
#else
#if OS_ANDROID
  ACQUIRE_PROC(AcquireNextImageKHR, handle);
  ACQUIRE_PROC(CreateSwapchainKHR, handle);
  ACQUIRE_PROC(DestroySwapchainKHR, handle);
  ACQUIRE_PROC(GetSwapchainImagesKHR, handle);
  ACQUIRE_PROC(QueuePresentKHR, handle);
#endif  // OS_ANDROID
#if OS_FUCHSIA
  ACQUIRE_PROC(GetMemoryZirconHandleFUCHSIA, handle);
  ACQUIRE_PROC(ImportSemaphoreZirconHandleFUCHSIA, handle);
#endif  // OS_FUCHSIA
#endif // RS_ENABLE_VK
  device_ = {handle, nullptr};
  return true;
}

bool VulkanProcTable::OpenLibraryHandle() {
#if VULKAN_LINK_STATICALLY
  static char kDummyLibraryHandle = '\0';
  handle_ = reinterpret_cast<decltype(handle_)>(&kDummyLibraryHandle);
  return true;
#else   // VULKAN_LINK_STATICALLY
  dlerror();  // clear existing errors on thread.
  handle_ = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
  if (handle_ == nullptr) {
#ifdef RS_ENABLE_VK
    LOGE("Could not open the vulkan library: %s", dlerror());
#else
    FML_DLOG(WARNING) << "Could not open the vulkan library: " << dlerror();
#endif
    return false;
  }
  return true;
#endif  // VULKAN_LINK_STATICALLY
}

bool VulkanProcTable::CloseLibraryHandle() {
#if VULKAN_LINK_STATICALLY
  handle_ = nullptr;
  return true;
#else
  if (handle_ != nullptr) {
    dlerror();  // clear existing errors on thread.
    if (dlclose(handle_) != 0) {
#ifdef RS_ENABLE_VK
      LOGE("Could not close the vulkan library handle. This "
                 "indicates a leak.");
      LOGE("%s", dlerror());
#else
      FML_DLOG(ERROR) << "Could not close the vulkan library handle. This "
                         "indicates a leak.";
      FML_DLOG(ERROR) << dlerror();
#endif
    }
    handle_ = nullptr;
  }
  return handle_ == nullptr;
#endif
}

PFN_vkVoidFunction VulkanProcTable::AcquireProc(
    const char* proc_name,
    const VulkanHandle<VkInstance>& instance) const {
  if (proc_name == nullptr || !GetInstanceProcAddr) {
    return nullptr;
  }

  // A VK_NULL_HANDLE as the instance is an acceptable parameter.
  return GetInstanceProcAddr(instance, proc_name);
}

PFN_vkVoidFunction VulkanProcTable::AcquireProc(
    const char* proc_name,
    const VulkanHandle<VkDevice>& device) const {
  if (proc_name == nullptr || !device || !GetDeviceProcAddr) {
    return nullptr;
  }

  return GetDeviceProcAddr(device, proc_name);
}

GrVkGetProc VulkanProcTable::CreateSkiaGetProc() const {
  if (!IsValid()) {
    return nullptr;
  }

  return [this](const char* proc_name, VkInstance instance, VkDevice device) {
    if (device != VK_NULL_HANDLE) {
      auto result = AcquireProc(proc_name, {device, nullptr});
      if (result != nullptr) {
        return result;
      }
    }

    return AcquireProc(proc_name, {instance, nullptr});
  };
}

}  // namespace vulkan
