//
// Created by Gianni on 20/11/2024.
//

#ifndef VULKAN3DMODELVIEWER_MESH_HPP
#define VULKAN3DMODELVIEWER_MESH_HPP

#include <array>
#include <vulkan/vulkan.h>
#include "../vk/vulkan_types.hpp"
#include "../vk/vulkan_functions.hpp"


struct Mesh
{
    VulkanBuffer vertexBuffer;
    IndexBuffer indexBuffer;
    VulkanTexture diffuseMap;
    VulkanTexture specularMap;
    VulkanTexture normalMap;
};

void destroyMesh(Mesh& mesh, VulkanRenderDevice& renderDevice);

void renderMesh(Mesh& mesh,
                VulkanRenderDevice& renderDevice,
                VkDescriptorSet descriptorSet,
                VkPipelineLayout pipelineLayout,
                VkCommandBuffer commandBuffer);
void updateDescriptorSet(Mesh& mesh, VulkanRenderDevice& renderDevice, VkDescriptorSet descriptorSet);

#endif //VULKAN3DMODELVIEWER_MESH_HPP
