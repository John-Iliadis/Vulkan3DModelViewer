#version 460 core

layout (location = 0) in vec2 vTexCoords;

layout (location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(vTexCoords, 0, 1.f);
}