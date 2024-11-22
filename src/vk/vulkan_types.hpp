//
// Created by Gianni on 17/11/2024.
//

#ifndef VULKAN3DMODELVIEWER_VULKAN_TYPES_HPP
#define VULKAN3DMODELVIEWER_VULKAN_TYPES_HPP

#include <vector>
#include <vulkan/vulkan.h>
#include <functional>


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

inline bool operator==(const VulkanTexture& left, const VulkanTexture& right)
{
    return (left.image.image == right.image.image &&
            left.image.imageView == right.image.imageView &&
            left.image.memory == right.image.memory &&
            left.sampler == right.sampler);
}

namespace std
{
    template<>
    struct hash<VulkanTexture>
    {
        size_t operator()(const VulkanTexture& key) const
        {
            return (((hash<VkImage>()(key.image.image) ^
                      hash<VkImageView>()(key.image.imageView) ^
                      hash<VkDeviceMemory>()(key.image.memory)) >> 1) ^
                    (hash<VkSampler>()(key.sampler)));
        }
    };
}


#endif //VULKAN3DMODELVIEWER_VULKAN_TYPES_HPP
