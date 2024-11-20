//
// Created by Gianni on 20/11/2024.
//

#ifndef VULKAN3DMODELVIEWER_VERTEX_HPP
#define VULKAN3DMODELVIEWER_VERTEX_HPP

#include <glm/glm.hpp>


struct Vertex
{
    glm::vec3 position;
    glm::vec2 texCoords;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

#endif //VULKAN3DMODELVIEWER_VERTEX_HPP
