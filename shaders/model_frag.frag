#version 460 core

layout (location = 0) in vec2 vTexCoords;

layout (location = 0) out vec4 outColor;

layout (set = 1, binding = 0) uniform sampler2D diffuseMap;
layout (set = 1, binding = 1) uniform sampler2D specularMap;
layout (set = 1, binding = 2) uniform sampler2D normalMap;


void main()
{
    outColor = texture(diffuseMap, vTexCoords);
}