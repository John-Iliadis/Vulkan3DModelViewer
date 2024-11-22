#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec3 bitangent;
layout (location = 4) in vec2 texCoords;

layout (location = 0) out vec2 vTexCoords;

layout (binding = 0) uniform UBO
{
    mat4 mvp;
} ubo;

void main()
{
    gl_Position = ubo.mvp * vec4(position, 1.f);
    vTexCoords = texCoords;
}