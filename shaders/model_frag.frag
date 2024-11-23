#version 460 core
#include "material.glsl"


layout (location = 0) in vec2 vTexCoords;

layout (location = 0) out vec4 outColor;

layout (push_constant) uniform PushConstants
{
    uint materialIndex;
};

layout (set = 1, binding = 0) readonly buffer MaterialSSBO
{
    Material materials[];
};

layout (set = 1, binding = 1) uniform sampler2D textures[46];

void main()
{
    Material material = materials[materialIndex];

    if (material.hasDiffuseMap == 1)
    {
        outColor = texture(textures[material.diffuseMapIndex], vTexCoords);
    }
    else
    {
        outColor = vec4(0.5, 0.5, 0.5, 1);
    }
}