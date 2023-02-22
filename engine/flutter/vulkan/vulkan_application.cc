// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/vulkan/vulkan_application.h"

#include <utility>
#include <vector>

#include "flutter/vulkan/vulkan_device.h"
#ifdef RS_ENABLE_VK
#include "flutter/vulkan/vulkan_hilog.h"
#endif
#include "flutter/vulkan/vulkan_proc_table.h"
#include "flutter/vulkan/vulkan_utilities.h"

namespace vulkan {

VulkanApplication::VulkanApplication(
    VulkanProcTable& p_vk,
    const std::string& application_name,
    std::vector<std::string> enabled_extensions,
    uint32_t application_version,
    uint32_t api_version)
    : vk(p_vk), api_version_(api_version), valid_(false) {
  // Check if we want to enable debugging.
  std::vector<VkExtensionProperties> supported_extensions =
      GetSupportedInstanceExtensions(vk);
  bool enable_instance_debugging =
      IsDebuggingEnabled() &&
      ExtensionSupported(supported_extensions,
                         VulkanDebugReport::DebugExtensionName());

  // Configure extensions.

  if (enable_instance_debugging) {
    enabled_extensions.emplace_back(VulkanDebugReport::DebugExtensionName());
  }
#if OS_FUCHSIA
  if (ExtensionSupported(supported_extensions,
                         VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME)) {
    // VK_KHR_get_physical_device_properties2 is a dependency of the memory
    // capabilities extension, so the validation layers require that it be
    // enabled.
    enabled_extensions.emplace_back(
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    enabled_extensions.emplace_back(
        VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);
  }
#endif

  const char* extensions[enabled_extensions.size()];

  for (size_t i = 0; i < enabled_extensions.size(); i++) {
    extensions[i] = enabled_extensions[i].c_str();
  }

  // Configure layers.

  const std::vector<std::string> enabled_layers = InstanceLayersToEnable(vk);

  const char* layers[enabled_layers.size()];

  for (size_t i = 0; i < enabled_layers.size(); i++) {
    layers[i] = enabled_layers[i].c_str();
  }

  // Configure init structs.

#ifdef RS_ENABLE_VK
  const VkApplicationInfo info = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext = nullptr,
      .pApplicationName = application_name.c_str(),
      .applicationVersion = application_version,
      .pEngineName = "Rosen",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = api_version_,
  };
#else
  const VkApplicationInfo info = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext = nullptr,
      .pApplicationName = application_name.c_str(),
      .applicationVersion = application_version,
      .pEngineName = "FlutterEngine",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = api_version_,
  };
#endif

  const VkInstanceCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .pApplicationInfo = &info,
      .enabledLayerCount = static_cast<uint32_t>(enabled_layers.size()),
      .ppEnabledLayerNames = layers,
      .enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size()),
      .ppEnabledExtensionNames = extensions,
  };

  // Perform initialization.

  VkInstance instance = VK_NULL_HANDLE;

  if (VK_CALL_LOG_ERROR(vk.CreateInstance(&create_info, nullptr, &instance)) !=
      VK_SUCCESS) {
#ifdef RS_ENABLE_VK
    LOGE("Could not create application instance.");
#else
    FML_DLOG(INFO) << "Could not create application instance.";
#endif
    return;
  }

  // Now that we have an instance, setup instance proc table entries.
  if (!vk.SetupInstanceProcAddresses(instance)) {
#ifdef RS_ENABLE_VK
    LOGE("Could not setup instance proc addresses.");
#else
    FML_DLOG(INFO) << "Could not setup instance proc addresses.";
#endif
    return;
  }

  instance_ = {instance, [this](VkInstance i) {
#ifdef RS_ENABLE_VK
                 LOGE("Destroying Vulkan instance");
#else
                 FML_LOG(INFO) << "Destroying Vulkan instance";
#endif
                 vk.DestroyInstance(i, nullptr);
               }};

  if (enable_instance_debugging) {
    auto debug_report = std::make_unique<VulkanDebugReport>(vk, instance_);
    if (!debug_report->IsValid()) {
#ifdef RS_ENABLE_VK
      LOGE("Vulkan debugging was enabled but could not be setup for this instance.");
#else
      FML_LOG(INFO) << "Vulkan debugging was enabled but could not be setup "
                       "for this instance.";
#endif
    } else {
      debug_report_ = std::move(debug_report);
#ifdef RS_ENABLE_VK
      LOGE("Debug reporting is enabled.");
#else
      FML_DLOG(INFO) << "Debug reporting is enabled.";
#endif
    }
  }

  valid_ = true;
}

VulkanApplication::~VulkanApplication() = default;

bool VulkanApplication::IsValid() const {
  return valid_;
}

uint32_t VulkanApplication::GetAPIVersion() const {
  return api_version_;
}

const VulkanHandle<VkInstance>& VulkanApplication::GetInstance() const {
  return instance_;
}

void VulkanApplication::ReleaseInstanceOwnership() {
  instance_.ReleaseOwnership();
}

std::vector<VkPhysicalDevice> VulkanApplication::GetPhysicalDevices() const {
  if (!IsValid()) {
    return {};
  }

  uint32_t device_count = 0;
  if (VK_CALL_LOG_ERROR(vk.EnumeratePhysicalDevices(instance_, &device_count,
                                                    nullptr)) != VK_SUCCESS) {
#ifdef RS_ENABLE_VK
    LOGE("Could not enumerate physical device.");
#else
    FML_DLOG(INFO) << "Could not enumerate physical device.";
#endif
    return {};
  }

  if (device_count == 0) {
    // No available devices.
#ifdef RS_ENABLE_VK
    LOGE("No physical devices found.");
#else
    FML_DLOG(INFO) << "No physical devices found.";
#endif
    return {};
  }

  std::vector<VkPhysicalDevice> physical_devices;

  physical_devices.resize(device_count);

  if (VK_CALL_LOG_ERROR(vk.EnumeratePhysicalDevices(
          instance_, &device_count, physical_devices.data())) != VK_SUCCESS) {
#ifdef RS_ENABLE_VK
    LOGE("Could not enumerate physical device.");
#else
    FML_DLOG(INFO) << "Could not enumerate physical device.";
#endif
    return {};
  }

  return physical_devices;
}

std::unique_ptr<VulkanDevice>
VulkanApplication::AcquireFirstCompatibleLogicalDevice() const {
  for (auto device_handle : GetPhysicalDevices()) {
    auto logical_device = std::make_unique<VulkanDevice>(vk, device_handle);
    if (logical_device->IsValid()) {
      return logical_device;
    }
  }
#ifdef RS_ENABLE_VK
  LOGE("Could not acquire compatible logical device.");
#else
  FML_DLOG(INFO) << "Could not acquire compatible logical device.";
#endif
  return nullptr;
}

std::vector<VkExtensionProperties>
VulkanApplication::GetSupportedInstanceExtensions(
    const VulkanProcTable& vk) const {
  if (!vk.EnumerateInstanceExtensionProperties) {
    return std::vector<VkExtensionProperties>();
  }

  uint32_t count = 0;
  if (VK_CALL_LOG_ERROR(vk.EnumerateInstanceExtensionProperties(
          nullptr, &count, nullptr)) != VK_SUCCESS) {
    return std::vector<VkExtensionProperties>();
  }

  if (count == 0) {
    return std::vector<VkExtensionProperties>();
  }

  std::vector<VkExtensionProperties> properties;
  properties.resize(count);
  if (VK_CALL_LOG_ERROR(vk.EnumerateInstanceExtensionProperties(
          nullptr, &count, properties.data())) != VK_SUCCESS) {
    return std::vector<VkExtensionProperties>();
  }

  return properties;
}

bool VulkanApplication::ExtensionSupported(
    const std::vector<VkExtensionProperties>& supported_instance_extensions,
    std::string extension_name) {
  uint32_t count = supported_instance_extensions.size();
  for (size_t i = 0; i < count; i++) {
    if (strncmp(supported_instance_extensions[i].extensionName,
                extension_name.c_str(), extension_name.size()) == 0) {
      return true;
    }
  }

  return false;
}

}  // namespace vulkan
