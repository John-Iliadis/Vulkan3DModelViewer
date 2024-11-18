//
// Created by Gianni on 17/11/2024.
//

#ifndef VULKAN3DMODELVIEWER_VULKAN_TYPES_HPP
#define VULKAN3DMODELVIEWER_VULKAN_TYPES_HPP

#include <vector>
#include <vulkan/vulkan.h>


struct VulkanInstance
{
    VkInstance instance;
    VkSurfaceKHR surface;

#ifdef DEBUG_MODE
    VkDebugUtilsMessengerEXT debugMessenger;
#endif
};

struct VulkanRenderDevice
{
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;

    VkSwapchainKHR swapchain;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;

    VkFormat format;
    VkExtent2D extent;
};

struct VulkanBuffer
{
    VkBuffer buffer;
    VkDeviceMemory memory;
};

struct VulkanTexture
{
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkSampler textureSampler;
};

#endif //VULKAN3DMODELVIEWER_VULKAN_TYPES_HPP
