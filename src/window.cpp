//
// Created by Gianni on 17/11/2024.
//

#include "window.hpp"


static constexpr int INITIAL_WINDOW_WIDTH = 1920;
static constexpr int INITIAL_WINDOW_HEIGHT = 1080;
static constexpr char* const WINDOW_TITLE = "3D Model Viewer";

Window::Window()
{
    initializeGLFW();
    createInstance(mInstance);

}

Window::~Window()
{
    destroyInstance(mInstance);
}

void Window::run()
{

}

void Window::initializeGLFW()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    mWindow = glfwCreateWindow(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);

    if (!mWindow)
    {
        throw std::runtime_error("Window::initializeGLFW: Failed to create GLFW window.");
    }
}
