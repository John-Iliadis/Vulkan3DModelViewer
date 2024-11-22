//
// Created by Gianni on 20/11/2024.
//

#include "mesh.hpp"


void destroyMesh(Mesh& mesh, VulkanRenderDevice& renderDevice)
{
    destroyBuffer(renderDevice, mesh.vertexBuffer);
    destroyIndexBuffer(renderDevice, mesh.indexBuffer);
}

void updateDescriptorSet(Mesh& mesh, VulkanRenderDevice& renderDevice, VkDescriptorSet descriptorSet)
{
    VkDescriptorImageInfo diffuseImageInfo {
        .sampler = mesh.diffuseMap.sampler,
        .imageView = mesh.diffuseMap.image.imageView,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    VkDescriptorImageInfo specularImageInfo {
        .sampler = mesh.specularMap.sampler,
        .imageView = mesh.specularMap.image.imageView,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    VkDescriptorImageInfo normalImageInfo {
        .sampler = mesh.normalMap.sampler,
        .imageView = mesh.normalMap.image.imageView,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    std::array<VkWriteDescriptorSet, 3> descriptorWrites;

    descriptorWrites.at(0) = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSet,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &diffuseImageInfo
    };

    descriptorWrites.at(1) = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSet,
        .dstBinding = 1,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &specularImageInfo
    };

    descriptorWrites.at(2) = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSet,
        .dstBinding = 2,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &normalImageInfo
    };

    vkUpdateDescriptorSets(renderDevice.device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

void renderMesh(Mesh& mesh,
                VulkanRenderDevice& renderDevice,
                VkDescriptorSet descriptorSet,
                VkPipelineLayout pipelineLayout,
                VkCommandBuffer commandBuffer)
{
    updateDescriptorSet(mesh, renderDevice, descriptorSet);
    vkCmdBindDescriptorSets(commandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout,
                            1, 1, &descriptorSet,
                            0, nullptr);

    VkDeviceSize offsets = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh.vertexBuffer.buffer, &offsets);
    vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer.buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(commandBuffer, mesh.indexBuffer.count, 1, 0, 0, 0);
}
