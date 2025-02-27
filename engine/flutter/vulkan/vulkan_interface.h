// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_VULKAN_VULKAN_INTERFACE_H_
#define FLUTTER_VULKAN_VULKAN_INTERFACE_H_

#include <string>

#ifndef RS_ENABLE_VK
#include "flutter/fml/build_config.h"
#include "flutter/fml/logging.h"
#endif

#ifndef RS_ENABLE_VK
#if !defined(FUCHSIA_SDK)
#define VULKAN_LINK_STATICALLY OS_FUCHSIA
#endif  //  !defined(FUCHSIA_SDK)

#if OS_ANDROID
#ifndef VK_USE_PLATFORM_ANDROID_KHR
#define VK_USE_PLATFORM_ANDROID_KHR 1
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#endif  // OS_ANDROID

#if OS_FUCHSIA
#ifndef VK_USE_PLATFORM_MAGMA_KHR
#define VK_USE_PLATFORM_MAGMA_KHR 1
#endif  // VK_USE_PLATFORM_MAGMA_KHR
#ifndef VK_USE_PLATFORM_FUCHSIA
#define VK_USE_PLATFORM_FUCHSIA 1
#endif  // VK_USE_PLATFORM_FUCHSIA
#endif  // OS_FUCHSIA
#endif  // not define RS_ENABLE_VK

#ifdef RS_ENABLE_VK
#ifndef VK_USE_PLATFORM_OHOS_OPENHARMONY
#define VK_USE_PLATFORM_OHOS_OPENHARMONY
#endif  // VK_USE_PLATFORM_OHOS_OPENHARMONY
#endif  // RS_ENABLE_VK

#if !VULKAN_LINK_STATICALLY
#define VK_NO_PROTOTYPES 1
#endif  // !VULKAN_LINK_STATICALLY

#include <vulkan/vulkan.h>

#ifndef NDEBUG

#ifdef RS_ENABLE_VK
#define VK_CALL_LOG_ERROR(expression)                            \
  ({                                                             \
    __typeof__(expression) _rc = (expression);                   \
    if (_rc != VK_SUCCESS) {                                     \
      LOGE("Vulkan call '" #expression "' failed with error %s", \
           vulkan::VulkanResultToString(_rc));                   \
    }                                                            \
    _rc;                                                         \
  })
#else
#define VK_CALL_LOG_ERROR(expression)                      \
  ({                                                       \
    __typeof__(expression) _rc = (expression);             \
    if (_rc != VK_SUCCESS) {                               \
      FML_DLOG(INFO) << "Vulkan call '" << #expression     \
                     << "' failed with error "             \
                     << vulkan::VulkanResultToString(_rc); \
    }                                                      \
    _rc;                                                   \
  })
#endif

#else  // NDEBUG

#define VK_CALL_LOG_ERROR(expression) (expression)

#endif  // NDEBUG

namespace vulkan {

std::string VulkanResultToString(VkResult result);

}  // namespace vulkan

#endif  // FLUTTER_VULKAN_VULKAN_INTERFACE_H_
