//
// Created by Gianni on 17/11/2024.
//

#ifndef VULKAN3DMODELVIEWER_VULKAN_FUNCTIONS_HPP
#define VULKAN3DMODELVIEWER_VULKAN_FUNCTIONS_HPP

#include <vector>
#include <optional>
#include <glfw/glfw3.h>
#include "vulkan_types.hpp"
#include "debug.hpp"


void createInstance(VulkanInstance& instance);
void destroyInstance(VulkanInstance& instance);

void createSurface(VulkanInstance& instance, GLFWwindow* window);

void createRenderingDevice(VulkanInstance& instance, VulkanRenderDevice& renderDevice);
void destroyRenderingDevice(VulkanRenderDevice& renderDevice);

std::vector<const char*> getInstanceExtensions();
std::vector<const char*> getDeviceExtensions();

void pickPhysicalDevice(VulkanInstance& instance, VulkanRenderDevice& device);
void createDevice(VulkanRenderDevice& renderDevice);
std::optional<uint32_t> findQueueFamilyIndex(VulkanRenderDevice& renderDevice, VkQueueFlags capabilitiesFlags);

void createSwapchain(VulkanInstance& instance, VulkanRenderDevice& renderDevice);
void createSwapchainImages(VulkanRenderDevice& renderDevice);

#endif //VULKAN3DMODELVIEWER_VULKAN_FUNCTIONS_HPP
