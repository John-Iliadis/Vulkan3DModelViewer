//
// Created by Gianni on 23/11/2024.
//

#ifndef VULKAN3DMODELVIEWER_MATERIAL_HPP
#define VULKAN3DMODELVIEWER_MATERIAL_HPP

#include <cstdint>

struct Material
{
    uint32_t diffuseMapIndex;
    uint32_t specularMapIndex;
    uint32_t normalMapIndex;
    int hasDiffuseMap;
    int hasSpecularMap;
    int hasNormalMap;
};

#endif //VULKAN3DMODELVIEWER_MATERIAL_HPP
