//
// Created by Gianni on 20/11/2024.
//

#include "mesh.hpp"


void createMesh(Mesh& mesh, VulkanBuffer& vertexBuffer, IndexBuffer& indexBuffer)
{
    mesh = {
        .vertexBuffer = vertexBuffer,
        .indexBuffer = indexBuffer
    };
}

void destroyMesh(Mesh& mesh, VulkanRenderDevice& renderDevice)
{
    destroyBuffer(renderDevice, mesh.vertexBuffer);
    destroyIndexBuffer(renderDevice, mesh.indexBuffer);
}

void renderMesh(Mesh& mesh, VkCommandBuffer commandBuffer)
{
    VkDeviceSize offsets = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh.vertexBuffer.buffer, &offsets);
    vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer.buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(commandBuffer, mesh.indexBuffer.count, 1, 0, 0, 0);
}
