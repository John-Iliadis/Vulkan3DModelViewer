//
// Created by Gianni on 20/11/2024.
//

#ifndef VULKAN3DMODELVIEWER_VERTEX_HPP
#define VULKAN3DMODELVIEWER_VERTEX_HPP

#include <glm/glm.hpp>


struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    glm::vec2 texCoords;

    static VkVertexInputBindingDescription bindingDescription()
    {
        return {
                .binding = 0,
                .stride = sizeof(Vertex),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        };
    }

    static std::vector<VkVertexInputAttributeDescription> attributeDescription()
    {
        return {
            {
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, position)
            },
            {
                .location = 1,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, normal)
            },
            {
                .location = 2,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, tangent)
            },
            {
                .location = 3,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, bitangent)
            },
            {
                .location = 4,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = offsetof(Vertex, texCoords)
            }
        };
    }
};


#endif //VULKAN3DMODELVIEWER_VERTEX_HPP
