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
    createCommandPool(renderDevice);
    createCommandBuffer(renderDevice);
    renderDevice.imageReadySemaphore = createSemaphore(renderDevice);
    renderDevice.renderFinishedSemaphore = createSemaphore(renderDevice);
}

void destroyRenderingDevice(VulkanRenderDevice& renderDevice)
{
    for (VkImageView imageView : renderDevice.swapchainImageViews)
    {
        vkDestroyImageView(renderDevice.device, imageView, nullptr);
    }

    vkDestroySemaphore(renderDevice.device, renderDevice.imageReadySemaphore, nullptr);
    vkDestroySemaphore(renderDevice.device, renderDevice.renderFinishedSemaphore, nullptr);
    vkDestroyCommandPool(renderDevice.device, renderDevice.commandPool, nullptr);
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

    vulkanCheck(static_cast<VkResult>(~VK_SUCCESS), "Failed to find a suitable physical device.");
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

    VkPhysicalDeviceFeatures physicalDeviceFeatures {
        .samplerAnisotropy = VK_TRUE
    };

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
    renderDevice.graphicsQueueFamilyIndex = queueFamilyIndex;
}

std::optional<uint32_t> findQueueFamilyIndex(VulkanRenderDevice& renderDevice, VkQueueFlags capabilitiesFlags)
{
    uint32_t queueFamilyPropertyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(renderDevice.physicalDevice, &queueFamilyPropertyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilyPropertiesVec(queueFamilyPropertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(renderDevice.physicalDevice,
                                             &queueFamilyPropertyCount,
                                             queueFamilyPropertiesVec.data());

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

    renderDevice.swapchainFormat = VK_FORMAT_R8G8B8A8_UNORM;
    renderDevice.swapchainExtent = surfaceCapabilities.currentExtent;

    VkSwapchainCreateInfoKHR swapchainCreateInfo {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = instance.surface,
        .minImageCount = 3,
        .imageFormat = renderDevice.swapchainFormat,
        .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        .imageExtent = renderDevice.swapchainExtent,
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
        renderDevice.swapchainImageViews.at(i) = createImageView(renderDevice,
                                                                 renderDevice.swapchainImages.at(i),
                                                                 renderDevice.swapchainFormat,
                                                                 VK_IMAGE_ASPECT_COLOR_BIT,
                                                                 1);
    }
}

void createCommandPool(VulkanRenderDevice& renderDevice)
{
    VkCommandPoolCreateInfo commandPoolCreateInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = renderDevice.graphicsQueueFamilyIndex
    };

    VkResult result = vkCreateCommandPool(renderDevice.device,
                                          &commandPoolCreateInfo,
                                          nullptr,
                                          &renderDevice.commandPool);

    vulkanCheck(result, "Failed to create command pool.");
}

void createCommandBuffer(VulkanRenderDevice& renderDevice)
{
    VkCommandBufferAllocateInfo commandBufferAllocateInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = renderDevice.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    VkResult result = vkAllocateCommandBuffers(renderDevice.device,
                                               &commandBufferAllocateInfo,
                                               &renderDevice.commandBuffer);
    vulkanCheck(result, "Failed to allocate command buffer.");
}

VkSemaphore createSemaphore(VulkanRenderDevice& renderDevice)
{
    VkSemaphore semaphore;

    VkSemaphoreCreateInfo semaphoreCreateInfo {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VkResult result = vkCreateSemaphore(renderDevice.device,
                                        &semaphoreCreateInfo,
                                        nullptr,
                                        &semaphore);
    vulkanCheck(result, "Failed to create semaphore.");

    return semaphore;
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

VulkanBuffer createBufferWithStaging(VulkanRenderDevice& renderDevice,
                                     VkDeviceSize size,
                                     VkBufferUsageFlags usage,
                                     void* bufferData)
{
    VkBufferUsageFlags stagingBufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VkMemoryPropertyFlags stagingBufferMemoryProperties {
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };

    VulkanBuffer stagingBuffer = createBuffer(renderDevice,
                                              size,
                                              stagingBufferUsage,
                                              stagingBufferMemoryProperties,
                                              bufferData);

    VkMemoryPropertyFlags bufferMemoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    VulkanBuffer buffer = createBuffer(renderDevice,
                                       size,
                                       usage,
                                       bufferMemoryProperties);

    copyBuffer(renderDevice, stagingBuffer, buffer, size);

    destroyBuffer(renderDevice, stagingBuffer);

    return buffer;
}

VulkanBuffer createVertexBuffer(VulkanRenderDevice& renderDevice, VkDeviceSize size, void* bufferData)
{
    return createBufferWithStaging(renderDevice,
                                   size,
                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                   bufferData);
}

IndexBuffer createIndexBuffer(VulkanRenderDevice& renderDevice, VkDeviceSize size, void* bufferData)
{
    IndexBuffer indexBuffer;

    indexBuffer.buffer = createBufferWithStaging(renderDevice,
                                                 size,
                                                 VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                 bufferData);
    indexBuffer.count = size / sizeof(uint32_t);

    return indexBuffer;
}

void destroyIndexBuffer(VulkanRenderDevice& renderDevice, IndexBuffer& indexBuffer)
{
    destroyBuffer(renderDevice, indexBuffer.buffer);
    indexBuffer = IndexBuffer();
}

void copyBuffer(VulkanRenderDevice& renderDevice, VulkanBuffer& srcBuffer, VulkanBuffer& dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = beginSingleCommand(renderDevice);

    VkBufferCopy copyRegion {
        .srcOffset {},
        .dstOffset {},
        .size = size
    };

    vkCmdCopyBuffer(commandBuffer, srcBuffer.buffer, dstBuffer.buffer, 1, &copyRegion);

    endSingleCommand(renderDevice, commandBuffer);
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

VkCommandBuffer beginSingleCommand(VulkanRenderDevice& renderDevice)
{
    VkCommandBuffer commandBuffer;

    VkCommandBufferAllocateInfo commandBufferAllocateInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = renderDevice.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    vkAllocateCommandBuffers(renderDevice.device, &commandBufferAllocateInfo, &commandBuffer);

    VkCommandBufferBeginInfo commandBufferBeginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

    return commandBuffer;
}

void endSingleCommand(VulkanRenderDevice& renderDevice, VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
    };

    vkQueueSubmit(renderDevice.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkDeviceWaitIdle(renderDevice.device);

    vkFreeCommandBuffers(renderDevice.device, renderDevice.commandPool, 1, &commandBuffer);
}

VulkanImage createImage(VulkanRenderDevice& renderDevice,
                        VkFormat format,
                        uint32_t width, uint32_t height,
                        VkImageUsageFlags usage,
                        VkImageAspectFlags aspectMask,
                        uint32_t mipLevels)
{
    VulkanImage image;

    // create image
    VkImageCreateInfo imageCreateInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = {.width = width, .height = height, .depth = 1},
        .mipLevels = mipLevels,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    VkResult result = vkCreateImage(renderDevice.device, &imageCreateInfo, nullptr, &image.image);
    vulkanCheck(result, "Failed to create image.");

    // create image memory
    VkMemoryRequirements imageMemoryRequirements;
    vkGetImageMemoryRequirements(renderDevice.device, image.image, &imageMemoryRequirements);

    VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    uint32_t imageMemoryTypeIndex = findSuitableMemoryType(renderDevice,
                                                           imageMemoryRequirements.memoryTypeBits,
                                                           memoryPropertyFlags).value();

    VkMemoryAllocateInfo memoryAllocateInfo {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = imageMemoryRequirements.size,
        .memoryTypeIndex = imageMemoryTypeIndex
    };

    result = vkAllocateMemory(renderDevice.device, &memoryAllocateInfo, nullptr, &image.memory);
    vulkanCheck(result, "Failed to allocate image memory.");

    vkBindImageMemory(renderDevice.device, image.image, image.memory, 0);

    // create image view
    image.imageView = createImageView(renderDevice, image.image, format, aspectMask, mipLevels);

    return image;
}

void destroyImage(VulkanRenderDevice& renderDevice, VulkanImage& image)
{
    vkDestroyImageView(renderDevice.device, image.imageView, nullptr);
    vkDestroyImage(renderDevice.device, image.image, nullptr);
    vkFreeMemory(renderDevice.device, image.memory, nullptr);
}

VkImageView createImageView(VulkanRenderDevice& renderDevice,
                            VkImage image,
                            VkFormat format,
                            VkImageAspectFlags aspectMask,
                            uint32_t mipLevels)
{
    VkImageView imageView;

    VkImageViewCreateInfo imageViewCreateInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange {
            .aspectMask = aspectMask,
            .baseMipLevel = 0,
            .levelCount = mipLevels,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    VkResult result = vkCreateImageView(renderDevice.device, &imageViewCreateInfo, nullptr, &imageView);
    vulkanCheck(result, "Failed to create image view.");

    return imageView;
}

void transitionImageLayout(VulkanRenderDevice& renderDevice,
                           VulkanImage& image,
                           VkImageLayout oldLayout,
                           VkImageLayout newLayout,
                           uint32_t mipLevels)
{
    VkCommandBuffer commandBuffer = beginSingleCommand(renderDevice);

    VkImageMemoryBarrier imageMemoryBarrier {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .image = image.image,
        .subresourceRange {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = mipLevels, // transition all mip levels
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    VkPipelineStageFlags srcStageMask {};
    VkPipelineStageFlags dstStageMask {};

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        vulkanCheck(static_cast<VkResult>(~VK_SUCCESS), "Operation not supported yet.");
    }

    vkCmdPipelineBarrier(commandBuffer,
                         srcStageMask,
                         dstStageMask,
                         0, 0, nullptr, 0, nullptr,
                         1, &imageMemoryBarrier);

    endSingleCommand(renderDevice, commandBuffer);
}

void copyBufferToImage(VulkanRenderDevice& renderDevice,
                       VulkanBuffer& buffer,
                       VulkanImage& image,
                       uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = beginSingleCommand(renderDevice);

    VkBufferImageCopy copyRegion {
        .bufferOffset = 0,
        .bufferRowLength = width,
        .bufferImageHeight = height,
        .imageSubresource {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
        .imageOffset {0, 0, 0},
        .imageExtent {.width = width, .height = height, .depth = 1},
    };

    vkCmdCopyBufferToImage(commandBuffer,
                           buffer.buffer,
                           image.image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1, &copyRegion);

    endSingleCommand(renderDevice, commandBuffer);
}

VulkanTexture createTexture(VulkanRenderDevice& renderDevice, const std::string& filename)
{
    VulkanTexture texture;

    // load image data
    int width, height;

    uint8_t* imageData = stbi_load(filename.c_str(), &width, &height, nullptr, STBI_rgb_alpha);
    vulkanCheck(static_cast<VkResult>(imageData ? VK_SUCCESS : ~VK_SUCCESS), "Failed to load image data.");

    VkDeviceSize size = width * height * 4;

    // create staging buffer
    VkMemoryPropertyFlags stagingBufferMemoryProperties {
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };

    VulkanBuffer stagingBuffer = createBuffer(renderDevice,
                                              size,
                                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                              stagingBufferMemoryProperties,
                                              imageData);

    stbi_image_free(imageData);

    // create image
    VkImageUsageFlags imageUsageFlags {
        VK_IMAGE_USAGE_TRANSFER_DST_BIT |
        VK_IMAGE_USAGE_SAMPLED_BIT
    };

    texture.image = createImage(renderDevice,
                                VK_FORMAT_R8G8B8A8_UNORM,
                                width, height,
                                imageUsageFlags,
                                VK_IMAGE_ASPECT_COLOR_BIT);

    // transition image layout for staging memory copy operation
    transitionImageLayout(renderDevice,
                          texture.image,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          1);

    // perform copy
    copyBufferToImage(renderDevice, stagingBuffer, texture.image, width, height);

    // transition image layout to shader read only
    transitionImageLayout(renderDevice,
                          texture.image,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                          1);

    // delete staging buffer
    destroyBuffer(renderDevice, stagingBuffer);

    // create sampler
    createSampler(renderDevice, texture, 1);

    return texture;
}

VulkanTexture createTextureWithMips(VulkanRenderDevice& renderDevice, const std::string& filename)
{
    VulkanTexture texture;

    // load image data
    int width, height;

    uint8_t* imageData = stbi_load(filename.c_str(), &width, &height, nullptr, STBI_rgb_alpha);
    vulkanCheck(static_cast<VkResult>(imageData ? VK_SUCCESS : ~VK_SUCCESS), "Failed to load image data.");

    VkDeviceSize size = width * height * 4;
    uint32_t mipLevels = glm::floor(glm::log2(glm::max(width, height))) + 1;

    // create staging buffer
    VkMemoryPropertyFlags stagingBufferMemoryProperties {
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };

    VulkanBuffer stagingBuffer = createBuffer(renderDevice,
                                              size,
                                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                              stagingBufferMemoryProperties,
                                              imageData);

    stbi_image_free(imageData);

    // create texture
    VkImageUsageFlags imageUsage {
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
        VK_IMAGE_USAGE_TRANSFER_DST_BIT |
        VK_IMAGE_USAGE_SAMPLED_BIT
    };

    texture.image = createImage(renderDevice,
                                VK_FORMAT_R8G8B8A8_UNORM,
                                width, height,
                                imageUsage,
                                VK_IMAGE_ASPECT_COLOR_BIT,
                                mipLevels);

    // transition image
    transitionImageLayout(renderDevice,
                          texture.image,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          mipLevels);

    // copy buffer to image
    copyBufferToImage(renderDevice, stagingBuffer, texture.image, width, height);

    // generate mips
    generateMipMaps(renderDevice, texture.image, width, height, mipLevels);

    // create sampler
    createSampler(renderDevice, texture, mipLevels);

    return texture;
}

void destroyTexture(VulkanRenderDevice& renderDevice, VulkanTexture& texture)
{
    destroyImage(renderDevice, texture.image);
    vkDestroySampler(renderDevice.device, texture.sampler, nullptr);
}

void createSampler(VulkanRenderDevice& renderDevice, VulkanTexture& texture, uint32_t mipLevels)
{
    VkBool32 anisotropyEnable = VK_FALSE;
    float maxAnisotropy = 0.f;

    if (mipLevels > 1)
    {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(renderDevice.physicalDevice, &physicalDeviceProperties);

        anisotropyEnable = VK_TRUE;
        maxAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy;
    }

    VkSamplerCreateInfo samplerCreateInfo {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = 0.f,
        .anisotropyEnable = anisotropyEnable,
        .maxAnisotropy = maxAnisotropy,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_NEVER,
        .minLod = 0.f,
        .maxLod = static_cast<float>(mipLevels - 1),
        .unnormalizedCoordinates = VK_FALSE
    };

    vkCreateSampler(renderDevice.device, &samplerCreateInfo, nullptr, &texture.sampler);
}

void generateMipMaps(VulkanRenderDevice& renderDevice,
                     VulkanImage& image,
                     uint32_t width, uint32_t height,
                     uint32_t mipLevels)
{
    VkCommandBuffer commandBuffer = beginSingleCommand(renderDevice);

    int32_t mipWidth = width;
    int32_t mipHeight = height;

    VkImageMemoryBarrier imageMemoryBarrier {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .image = image.image,
        .subresourceRange {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    for (uint32_t i = 0; i < mipLevels - 1; ++i)
    {
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        imageMemoryBarrier.subresourceRange.baseMipLevel = i;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0, 0, nullptr, 0, nullptr,
                             1, &imageMemoryBarrier);

        VkImageBlit blitRegion {
            .srcSubresource {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = i,
                .baseArrayLayer = 0,
                .layerCount = 1
            },
            .srcOffsets {
                {0, 0, 0},
                {mipWidth, mipHeight, 1}
            },
            .dstSubresource {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = i + 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            },
            .dstOffsets {
                {0, 0, 0},
                {mipWidth / 2, mipHeight / 2, 1}
            }
        };

        mipWidth /= 2;
        mipHeight /= 2;

        vkCmdBlitImage(commandBuffer,
                       image.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &blitRegion,
                       VK_FILTER_LINEAR);

        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             0, 0, nullptr, 0, nullptr,
                             1, &imageMemoryBarrier);
    }

    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageMemoryBarrier.subresourceRange.baseMipLevel = mipLevels - 1;

    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0, 0, nullptr, 0, nullptr,
                         1, &imageMemoryBarrier);

    endSingleCommand(renderDevice, commandBuffer);
}

VkShaderModule createShaderModule(VulkanRenderDevice& renderDevice, const std::string& filename)
{
    std::ifstream spirv(filename, std::ios::binary | std::ios::ate);

    if (!spirv.is_open())
        vulkanCheck(static_cast<VkResult>(~VK_SUCCESS), "File not open");

    size_t byteCount = spirv.tellg();
    char* code = new char[byteCount];
    spirv.seekg(0);
    spirv.read(code, byteCount);

    VkShaderModule shaderModule;

    VkShaderModuleCreateInfo shaderModuleCreateInfo {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = byteCount,
        .pCode = reinterpret_cast<uint32_t*>(code)
    };

    VkResult result = vkCreateShaderModule(renderDevice.device, &shaderModuleCreateInfo, nullptr, &shaderModule);
    vulkanCheck(result, "Failed to create shader module.");

    delete[] code;

    return shaderModule;
}
