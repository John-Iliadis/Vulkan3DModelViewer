//
// Created by Gianni on 17/11/2024.
//

#include "vulkan_functions.hpp"


void createInstance(VulkanInstance& instance)
{
    std::vector<const char*> extensions {
        "VK_KHR_surface",
        "VK_KHR_win32_surface"
    };

#ifdef DEBUG_MODE
    std::vector<const char*> layers {
        "VK_LAYER_KHRONOS_validation"
    };

    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    VkApplicationInfo applicationInfo {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Vulkan3DModelViewer",
        .applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
        .pEngineName = nullptr,
        .apiVersion = VK_API_VERSION_1_1
    };

    VkInstanceCreateInfo instanceCreateInfo {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &applicationInfo,
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data()
    };

#ifdef DEBUG_MODE
    VkDebugUtilsMessageSeverityFlagsEXT debugUtilsMessageSeverityFlags {
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
    };

    VkDebugUtilsMessageTypeFlagsEXT debugUtilsMessageTypeFlags {
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
    };

    VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = debugUtilsMessageSeverityFlags,
        .messageType = debugUtilsMessageTypeFlags,
        .pfnUserCallback = debugCallback,
        .pUserData = nullptr
    };

    instanceCreateInfo.enabledLayerCount = layers.size();
    instanceCreateInfo.ppEnabledLayerNames = layers.data();
    instanceCreateInfo.pNext = &debugUtilsMessengerCreateInfo;
#endif

    // create vulkan instance
    VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance.instance);
    vulkanCheck(result, "Failed to create Vulkan instance");

#ifdef DEBUG_MODE
    // create the debug messenger
    auto funcPtr = vkGetInstanceProcAddr(instance.instance, "vkCreateDebugUtilsMessengerEXT");
    auto vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(funcPtr);

    result = vkCreateDebugUtilsMessengerEXT(instance.instance,
                                       &debugUtilsMessengerCreateInfo,
                                       nullptr,
                                       &instance.debugMessenger);
    vulkanCheck(result, "Failed to create debug messenger");
#endif
}

void destroyInstance(VulkanInstance& instance)
{
#ifdef DEBUG_MODE
    auto funcPtr = vkGetInstanceProcAddr(instance.instance, "vkDestroyDebugUtilsMessengerEXT");
    auto vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(funcPtr);
    vkDestroyDebugUtilsMessengerEXT(instance.instance, instance.debugMessenger, nullptr);
#endif
    vkDestroyInstance(instance.instance, nullptr);
}
