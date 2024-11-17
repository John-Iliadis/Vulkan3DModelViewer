//
// Created by Gianni on 17/11/2024.
//

#ifndef VULKAN3DMODELVIEWER_VULKAN_TYPES_HPP
#define VULKAN3DMODELVIEWER_VULKAN_TYPES_HPP

#include <vulkan/vulkan.h>


struct VulkanInstance
{
    VkInstance instance;

#ifdef DEBUG_MODE
    VkDebugUtilsMessengerEXT debugMessenger;
#endif
};

struct RenderingDevice
{
    VkPhysicalDevice gpu;
    VkDevice device;
};

#endif //VULKAN3DMODELVIEWER_VULKAN_TYPES_HPP
