//
// Created by Gianni on 20/11/2024.
//

#ifndef VULKAN3DMODELVIEWER_MESH_HPP
#define VULKAN3DMODELVIEWER_MESH_HPP

#include <vulkan/vulkan.h>
#include "../vk/vulkan_types.hpp"
#include "../vk/vulkan_functions.hpp"


struct Mesh
{
    VulkanBuffer vertexBuffer;
    IndexBuffer indexBuffer;
};

void createMesh(Mesh& mesh, VulkanBuffer& vertexBuffer, IndexBuffer& indexBuffer);
void destroyMesh(Mesh& mesh, VulkanRenderDevice& renderDevice);

void renderMesh(Mesh& mesh, VkCommandBuffer commandBuffer);

#endif //VULKAN3DMODELVIEWER_MESH_HPP
