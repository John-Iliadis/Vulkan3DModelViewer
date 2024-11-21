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
    VkDebugUtilsMessengerEXT debugMessenger;
};

struct VulkanRenderDevice
{
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;

    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    uint32_t graphicsQueueFamilyIndex;

    VkSemaphore imageReadySemaphore;
    VkSemaphore renderFinishedSemaphore;

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

struct IndexBuffer
{
    VulkanBuffer buffer;
    uint32_t count;
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
