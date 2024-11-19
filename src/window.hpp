//
// Created by Gianni on 17/11/2024.
//

#ifndef VULKAN3DMODELVIEWER_WINDOW_HPP
#define VULKAN3DMODELVIEWER_WINDOW_HPP

#include <array>
#include <vulkan/vulkan.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include "vk/vulkan_types.hpp"
#include "vk/vulkan_functions.hpp"

struct Vertex
{
    glm::vec3 position;
    glm::vec2 texCoords;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription vertexInputBindingDescription {
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        };

        return vertexInputBindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions;

        attributeDescriptions.at(0) = {
            .location = 0,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, position)
        };

        attributeDescriptions.at(1) = {
            .location = 2,
            .binding = 0,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = offsetof(Vertex, texCoords)
        };

        return attributeDescriptions;
    }
};

class Window
{
public:
    Window();
    ~Window();

    void run();

private:
    void initializeGLFW();
    void renderFrame();

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
    GLFWwindow* mWindow;
    VulkanInstance mInstance;
    VulkanRenderDevice mRenderDevice;
};


#endif //VULKAN3DMODELVIEWER_WINDOW_HPP
