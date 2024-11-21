//
// Created by Gianni on 17/11/2024.
//

#ifndef VULKAN3DMODELVIEWER_VULKAN_FUNCTIONS_HPP
#define VULKAN3DMODELVIEWER_VULKAN_FUNCTIONS_HPP

#include <vector>
#include <optional>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/integer.hpp>
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

void createCommandPool(VulkanRenderDevice& renderDevice);

VulkanBuffer createBuffer(VulkanRenderDevice& renderDevice,
                          VkDeviceSize size,
                          VkBufferUsageFlags usage,
                          VkMemoryPropertyFlags memoryProperties);
VulkanBuffer createBuffer(VulkanRenderDevice& renderDevice,
                          VkDeviceSize size,
                          VkBufferUsageFlags usage,
                          VkMemoryPropertyFlags memoryProperties,
                          void* bufferData);

void destroyBuffer(VulkanRenderDevice& renderDevice, VulkanBuffer& buffer);

VulkanBuffer createBufferWithStaging(VulkanRenderDevice& renderDevice,
                                     VkDeviceSize size,
                                     VkBufferUsageFlags usage,
                                     void* bufferData);

VulkanBuffer createVertexBuffer(VulkanRenderDevice& renderDevice, VkDeviceSize size, void* bufferData);
VulkanBuffer createIndexBuffer(VulkanRenderDevice& renderDevice, VkDeviceSize size, void* bufferData);

void copyBuffer(VulkanRenderDevice& renderDevice, VulkanBuffer& srcBuffer, VulkanBuffer& dstBuffer, VkDeviceSize size);

std::optional<uint32_t> findSuitableMemoryType(VulkanRenderDevice& renderDevice,
                                               uint32_t resourceSupportedMemoryTypes,
                                               VkMemoryPropertyFlags desiredMemoryProperties);

VkCommandBuffer beginSingleCommand(VulkanRenderDevice& renderDevice);
void endSingleCommand(VulkanRenderDevice& renderDevice, VkCommandBuffer commandBuffer);

VulkanImage createImage(VulkanRenderDevice& renderDevice,
                        VkFormat format,
                        uint32_t width, uint32_t height,
                        VkImageUsageFlags usage,
                        VkImageAspectFlags aspectMask,
                        uint32_t mipLevels = 1);
void destroyImage(VulkanRenderDevice& renderDevice, VulkanImage& image);

VkImageView createImageView(VulkanRenderDevice& renderDevice,
                            VkImage image,
                            VkFormat format,
                            VkImageAspectFlags aspectMask,
                            uint32_t mipLevels);

void transitionImageLayout(VulkanRenderDevice& renderDevice,
                           VulkanImage& image,
                           VkImageLayout oldLayout,
                           VkImageLayout newLayout,
                           uint32_t mipLevels);

void copyBufferToImage(VulkanRenderDevice& renderDevice,
                       VulkanBuffer& buffer,
                       VulkanImage& image,
                       uint32_t width, uint32_t height);

VulkanTexture createTexture(VulkanRenderDevice& renderDevice, const std::string& filename);
VulkanTexture createTextureWithMips(VulkanRenderDevice& renderDevice, const std::string& filename);
void destroyTexture(VulkanRenderDevice& renderDevice, VulkanTexture& texture);
void createSampler(VulkanRenderDevice& renderDevice, VulkanTexture& texture, uint32_t mipLevels = 1);
void generateMipMaps(VulkanRenderDevice& renderDevice,
                     VulkanImage& image,
                     uint32_t width, uint32_t height,
                     uint32_t mipLevels);

#endif //VULKAN3DMODELVIEWER_VULKAN_FUNCTIONS_HPP
