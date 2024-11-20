//
// Created by Gianni on 20/11/2024.
//

#include "mesh.hpp"


void createMesh(Mesh& mesh, VulkanBuffer vertexBuffer, VulkanBuffer indexBuffer)
{
    mesh = {
        .vertexBuffer = vertexBuffer,
        .indexBuffer = indexBuffer
    };
}

void destroyMesh(Mesh& mesh, VulkanRenderDevice& renderDevice)
{
    destroyBuffer(renderDevice, mesh.vertexBuffer);
    destroyBuffer(renderDevice, mesh.indexBuffer);
}

void renderMesh(Mesh& mesh, VkCommandBuffer commandBuffer)
{

}
