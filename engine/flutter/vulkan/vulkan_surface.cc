// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/vulkan/vulkan_surface.h"

#include "flutter/vulkan/vulkan_application.h"
#ifdef RS_ENABLE_VK
#include "flutter/vulkan/vulkan_hilog.h"
#endif
#include "flutter/vulkan/vulkan_native_surface.h"

namespace vulkan {

VulkanSurface::VulkanSurface(
    VulkanProcTable& p_vk,
    VulkanApplication& application,
    std::unique_ptr<VulkanNativeSurface> native_surface)
    : vk(p_vk),
      application_(application),
      native_surface_(std::move(native_surface)),
      valid_(false) {
  if (native_surface_ == nullptr || !native_surface_->IsValid()) {
#ifdef RS_ENABLE_VK
    LOGE("Native surface was invalid.");
#else
    FML_DLOG(INFO) << "Native surface was invalid.";
#endif
    return;
  }

  VkSurfaceKHR surface =
      native_surface_->CreateSurfaceHandle(vk, application.GetInstance());

  if (surface == VK_NULL_HANDLE) {
#ifdef RS_ENABLE_VK
    LOGE("Could not create the surface handle.");
#else
    FML_DLOG(INFO) << "Could not create the surface handle.";
#endif
    return;
  }

  surface_ = {surface, [this](VkSurfaceKHR surface) {
                vk.DestroySurfaceKHR(application_.GetInstance(), surface,
                                     nullptr);
              }};

  valid_ = true;
}

VulkanSurface::~VulkanSurface() = default;

bool VulkanSurface::IsValid() const {
  return valid_;
}

const VulkanHandle<VkSurfaceKHR>& VulkanSurface::Handle() const {
  return surface_;
}

const VulkanNativeSurface& VulkanSurface::GetNativeSurface() const {
  return *native_surface_;
}

SkISize VulkanSurface::GetSize() const {
  return valid_ ? native_surface_->GetSize() : SkISize::Make(0, 0);
}

}  // namespace vulkan
