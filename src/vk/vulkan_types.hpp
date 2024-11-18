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
    VkCommandPool commandPool;
    uint32_t graphicsQueueFamilyIndex;

    VkSwapchainKHR swapchain;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    VkFormat swapchainFormat;
    VkExtent2D swapchainExtent;
};

struct VulkanBuffer
{
    VkBuffer buffer;
    VkDeviceMemory memory;
};

struct VulkanImage
{
    VkImage image;
    VkImageView imageView;
    VkDeviceMemory memory;
};

struct VulkanTexture
{
    VulkanImage image;
    VkSampler sampler;
};

#endif //VULKAN3DMODELVIEWER_VULKAN_TYPES_HPP
