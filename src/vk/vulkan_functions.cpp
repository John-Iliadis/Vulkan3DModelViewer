//
// Created by Gianni on 17/11/2024.
//

#include <stb/stb_image.h>
#include "vulkan_functions.hpp"


void createInstance(VulkanInstance& instance)
{
    std::vector<const char*> extensions = getInstanceExtensions();

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

    const char* validationLayer = "VK_LAYER_KHRONOS_validation";
    instanceCreateInfo.enabledLayerCount = 1;
    instanceCreateInfo.ppEnabledLayerNames = &validationLayer;
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
    vkDestroySurfaceKHR(instance.instance, instance.surface, nullptr);
    vkDestroyInstance(instance.instance, nullptr);
}

void createSurface(VulkanInstance& instance, GLFWwindow* window)
{
    VkResult result = glfwCreateWindowSurface(instance.instance, window, nullptr, &instance.surface);
    vulkanCheck(result, "Failed to create surface");
}

void createRenderingDevice(VulkanInstance& instance, VulkanRenderDevice& renderDevice)
{
    pickPhysicalDevice(instance, renderDevice);
    createDevice(renderDevice);
    createSwapchain(instance, renderDevice);
    createSwapchainImages(renderDevice);
}

void destroyRenderingDevice(VulkanRenderDevice& renderDevice)
{
    for (VkImageView imageView : renderDevice.swapchainImageViews)
    {
        vkDestroyImageView(renderDevice.device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(renderDevice.device, renderDevice.swapchain, nullptr);
    vkDestroyDevice(renderDevice.device, nullptr);
}

std::vector<const char*> getInstanceExtensions()
{
    std::vector<const char*> extensions {
            "VK_KHR_surface",
            "VK_KHR_win32_surface"
    };

#ifdef DEBUG_MODE
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    return extensions;
}

std::vector<const char*> getDeviceExtensions()
{
    std::vector<const char*> extensions {
            "VK_KHR_swapchain"
    };

    return extensions;
}

void pickPhysicalDevice(VulkanInstance& instance, VulkanRenderDevice& device)
{
    uint32_t physicalDeviceCount;
    vkEnumeratePhysicalDevices(instance.instance, &physicalDeviceCount, nullptr);

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(instance.instance, &physicalDeviceCount, physicalDevices.data());

    static auto searchPhysicalDeviceType = [&] (VkPhysicalDeviceType type) {
        for (VkPhysicalDevice physicalDevice : physicalDevices)
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(physicalDevice, &properties);

            if (properties.deviceType == type)
            {
                device.physicalDevice = physicalDevice;
                return true;
            }
        }

        return false;
    };

    bool foundDiscreteGPU = searchPhysicalDeviceType(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);

    if (foundDiscreteGPU)
        return;

    bool foundIntegratedGPU = searchPhysicalDeviceType(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);

    if (foundIntegratedGPU)
        return;

    vulkanCheck(VK_INCOMPLETE, "Failed to find a suitable physical device");
}

void createDevice(VulkanRenderDevice& renderDevice)
{
    uint32_t queueFamilyIndex = findQueueFamilyIndex(renderDevice, VK_QUEUE_GRAPHICS_BIT).value();

    float queuePriority = 1.f;
    VkDeviceQueueCreateInfo queueCreateInfo {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = queueFamilyIndex,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
    };

    std::vector<const char*> extensions = getDeviceExtensions();

    VkPhysicalDeviceFeatures physicalDeviceFeatures {};

    VkDeviceCreateInfo deviceCreateInfo {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queueCreateInfo,
            .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data(),
            .pEnabledFeatures = &physicalDeviceFeatures
    };

    VkResult result = vkCreateDevice(renderDevice.physicalDevice, &deviceCreateInfo, nullptr, &renderDevice.device);

    vulkanCheck(result, "Failed to create logical device.");

    vkGetDeviceQueue(renderDevice.device, queueFamilyIndex, 0, &renderDevice.graphicsQueue);
}

std::optional<uint32_t> findQueueFamilyIndex(VulkanRenderDevice& renderDevice, VkQueueFlags capabilitiesFlags)
{
    uint32_t queueFamilyPropertyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(renderDevice.physicalDevice, &queueFamilyPropertyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilyPropertiesVec(queueFamilyPropertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(renderDevice.physicalDevice, &queueFamilyPropertyCount, queueFamilyPropertiesVec.data());

    for (uint32_t i = 0, size = queueFamilyPropertiesVec.size(); i < size; ++i)
    {
        if (queueFamilyPropertiesVec.at(i).queueFlags & capabilitiesFlags)
            return i;
    }

    return {};
}

void createSwapchain(VulkanInstance& instance, VulkanRenderDevice& renderDevice)
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(renderDevice.physicalDevice, instance.surface, &surfaceCapabilities);

    renderDevice.format = VK_FORMAT_R8G8B8A8_UNORM;
    renderDevice.extent = surfaceCapabilities.currentExtent;

    VkSwapchainCreateInfoKHR swapchainCreateInfo {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = instance.surface,
        .minImageCount = 3,
        .imageFormat = renderDevice.format,
        .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        .imageExtent = renderDevice.extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = surfaceCapabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR,
        .clipped = VK_TRUE
    };

    VkResult result = vkCreateSwapchainKHR(renderDevice.device, &swapchainCreateInfo, nullptr, &renderDevice.swapchain);
    vulkanCheck(result, "Failed to create swapchain.");
}

void createSwapchainImages(VulkanRenderDevice& renderDevice)
{
    uint32_t imageCount;
    vkGetSwapchainImagesKHR(renderDevice.device, renderDevice.swapchain, &imageCount, nullptr);

    renderDevice.swapchainImages.resize(imageCount);
    renderDevice.swapchainImageViews.resize(imageCount);

    vkGetSwapchainImagesKHR(renderDevice.device,
                            renderDevice.swapchain,
                            &imageCount,
                            renderDevice.swapchainImages.data());

    for (size_t i = 0, size = renderDevice.swapchainImages.size(); i < size; ++i)
    {
        VkImageViewCreateInfo imageViewCreateInfo {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = renderDevice.swapchainImages.at(i),
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = renderDevice.format,
            .subresourceRange {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        VkResult result = vkCreateImageView(renderDevice.device,
                                            &imageViewCreateInfo,
                                            nullptr,
                                            &renderDevice.swapchainImageViews.at(i));

        vulkanCheck(result, "Failed to create a swapchain image view.");
    }
}

VulkanBuffer createBuffer(VulkanRenderDevice& renderDevice,
                          VkDeviceSize size,
                          VkBufferUsageFlags usage,
                          VkMemoryPropertyFlags memoryProperties)
{
    VkBuffer buffer;
    VkDeviceMemory bufferMemory;

    VkBufferCreateInfo bufferCreateInfo {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VkResult result = vkCreateBuffer(renderDevice.device, &bufferCreateInfo, nullptr, &buffer);
    vulkanCheck(result, "Failed to create staging buffer.");

    VkMemoryRequirements stagingBufferMemoryRequirements;
    vkGetBufferMemoryRequirements(renderDevice.device, buffer, &stagingBufferMemoryRequirements);

    uint32_t stagingBufferMemoryTypeIndex = findSuitableMemoryType(renderDevice,
                                                                   stagingBufferMemoryRequirements.memoryTypeBits,
                                                                   memoryProperties).value();

    VkMemoryAllocateInfo memoryAllocateInfo {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = stagingBufferMemoryRequirements.size,
            .memoryTypeIndex = stagingBufferMemoryTypeIndex
    };

    result = vkAllocateMemory(renderDevice.device, &memoryAllocateInfo, nullptr, &bufferMemory);
    vulkanCheck(result, "Failed to allocate staging buffer memory.");

    vkBindBufferMemory(renderDevice.device, buffer, bufferMemory, 0);

    return VulkanBuffer(buffer, bufferMemory);
}

// overload that maps the buffer data
VulkanBuffer createBuffer(VulkanRenderDevice& renderDevice,
                          VkDeviceSize size,
                          VkBufferUsageFlags usage,
                          VkMemoryPropertyFlags memoryProperties,
                          void* bufferData)
{
    VulkanBuffer buffer = createBuffer(renderDevice, size, usage, memoryProperties);

    void* dataPtr;
    vkMapMemory(renderDevice.device, buffer.memory, 0, size, 0, &dataPtr);
    memcpy(dataPtr, bufferData, size);
    vkUnmapMemory(renderDevice.device, buffer.memory);

    return buffer;
}

void destroyBuffer(VulkanRenderDevice& renderDevice, VulkanBuffer& buffer)
{
    vkDestroyBuffer(renderDevice.device, buffer.buffer, nullptr);
    vkFreeMemory(renderDevice.device, buffer.memory, nullptr);
}

void createTexture(VulkanTexture& texture, const std::string& filename, VulkanRenderDevice& renderDevice)
{
    int width, height;

    uint8_t* data = stbi_load(filename.c_str(), &width, &height, nullptr, STBI_rgb_alpha);

    vulkanCheck(static_cast<VkResult>(data ? VK_SUCCESS : ~VK_SUCCESS), "Failed to load image data.");


}

void createImage(VulkanTexture& texture,
                 VulkanRenderDevice& renderDevice,
                 uint8_t* imageData,
                 VkFormat format,
                 VkDeviceSize size,
                 uint32_t width, uint32_t height)
{
    // todo: check this flag
    VkBufferUsageFlags stagingBufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VkMemoryPropertyFlags stagingBufferMemoryProperties {
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };

    VulkanBuffer stagingBuffer = createBuffer(renderDevice,
                                              size,
                                              stagingBufferUsageFlags,
                                              stagingBufferMemoryProperties,
                                              imageData);
}

std::optional<uint32_t> findSuitableMemoryType(VulkanRenderDevice& renderDevice,
                                               uint32_t resourceSupportedMemoryTypes,
                                               VkMemoryPropertyFlags desiredMemoryProperties)
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(renderDevice.physicalDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
    {
        if (resourceSupportedMemoryTypes & (1 << i) &&
            (desiredMemoryProperties & memoryProperties.memoryTypes[i].propertyFlags) == desiredMemoryProperties)
            return i;
    }

    return {};
}
