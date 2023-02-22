// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/vulkan/vulkan_native_surface_ohos.h"

#include "flutter/vulkan/vulkan_hilog.h"
#include "window.h"
#include "third_party/skia/include/gpu/vk/GrVkBackendContext.h"
#include <graphic_common.h>

namespace vulkan {

VulkanNativeSurfaceOHOS::VulkanNativeSurfaceOHOS(struct NativeWindow* native_window) : native_window_(native_window)
{
    if (native_window_ == nullptr) {
        return;
    }
}

VulkanNativeSurfaceOHOS::~VulkanNativeSurfaceOHOS()
{
    if (native_window_ == nullptr) {
        return;
    }
}

const char* VulkanNativeSurfaceOHOS::GetExtensionName() const
{
    return VK_OPENHARMONY_OHOS_SURFACE_EXTENSION_NAME;
}

uint32_t VulkanNativeSurfaceOHOS::GetSkiaExtensionName() const
{
    return kKHR_ohos_surface_GrVkExtensionFlag;
}

VkSurfaceKHR VulkanNativeSurfaceOHOS::CreateSurfaceHandle(
    VulkanProcTable& vk, const VulkanHandle<VkInstance>& instance) const
{
    if (!vk.IsValid() || !instance) {
        LOGE("CreateSurfaceHandle vk or instance is not valid");
        return VK_NULL_HANDLE;
    }

    const VkOHOSSurfaceCreateInfoOpenHarmony create_info = {
        .sType = VK_STRUCTURE_TYPE_OHOS_SURFACE_CREATE_INFO_OPENHARMONY,
        .pNext = nullptr,
        .flags = 0,
        .window = native_window_,
    };

    VkSurfaceKHR surface = VK_NULL_HANDLE;

    if (VK_CALL_LOG_ERROR(vk.CreateOHOSSurfaceOpenHarmony(instance, &create_info, nullptr, &surface)) != VK_SUCCESS) {
        LOGE("CreateSurfaceHandle CreateOHOSSurfaceKHR not success");
        return VK_NULL_HANDLE;
    }

    return surface;
}

bool VulkanNativeSurfaceOHOS::IsValid() const
{
    return native_window_ != nullptr;
}

SkISize VulkanNativeSurfaceOHOS::GetSize() const
{
    int width, height;
    int err = NativeWindowHandleOpt(native_window_, GET_BUFFER_GEOMETRY, &height, &width);
    return native_window_ == nullptr || err != OHOS::GSERROR_OK ? SkISize::Make(0, 0) : SkISize::Make(width, height);
}

} // namespace vulkan