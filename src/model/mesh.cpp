//
// Created by Gianni on 20/11/2024.
//

#include "mesh.hpp"


void destroyMesh(Mesh& mesh, VulkanRenderDevice& renderDevice)
{
    destroyBuffer(renderDevice, mesh.vertexBuffer);
    destroyIndexBuffer(renderDevice, mesh.indexBuffer);
}

void renderMesh(Mesh& mesh, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh.vertexBuffer.buffer, &offset);
    vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer.buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32_t), &mesh.materialIndex);
    vkCmdDrawIndexed(commandBuffer, mesh.indexBuffer.count, 1, 0, 0, 0);
}