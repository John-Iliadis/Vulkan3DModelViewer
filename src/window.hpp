//
// Created by Gianni on 17/11/2024.
//

#ifndef VULKAN3DMODELVIEWER_WINDOW_HPP
#define VULKAN3DMODELVIEWER_WINDOW_HPP

#include <vulkan/vulkan.h>
#include <glfw/glfw3.h>
#include "vk/vulkan_types.hpp"
#include "vk/vulkan_functions.hpp"


class Window
{
public:
    Window();
    ~Window();

    void run();

private:
    void initializeGLFW();

    GLFWwindow* mWindow;
    VulkanInstance mInstance;
};


#endif //VULKAN3DMODELVIEWER_WINDOW_HPP
