//
// Created by Gianni on 17/11/2024.
//

#include "window.hpp"


static constexpr int INITIAL_WINDOW_WIDTH = 1920;
static constexpr int INITIAL_WINDOW_HEIGHT = 1080;
static constexpr char* const WINDOW_TITLE = "3D Model Viewer";


Window::Window()
    : mInstance()
    , mRenderDevice()
{
    initializeGLFW();
    createInstance(mInstance);
    createSurface(mInstance, mWindow);
    createRenderingDevice(mInstance, mRenderDevice);
    createDescriptorPool();
}

Window::~Window()
{
    vkDestroyDescriptorPool(mRenderDevice.device, mDescriptorPool, nullptr);
    destroyRenderingDevice(mRenderDevice);
    destroyInstance(mInstance);
    glfwTerminate();
}

void Window::run()
{
    while (!glfwWindowShouldClose(mWindow))
    {
        glfwPollEvents();
        renderFrame();
    }

     vkDeviceWaitIdle(mRenderDevice.device);
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

    glfwSetWindowUserPointer(mWindow, this);
    glfwSetKeyCallback(mWindow, keyCallback);
}

void Window::renderFrame()
{
}

void Window::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    Window& appWindow = *reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));

    if (key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void Window::createDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> descriptorPoolSizes {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2}
    };

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 2,
        .poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size()),
        .pPoolSizes = descriptorPoolSizes.data()
    };

    VkResult result = vkCreateDescriptorPool(mRenderDevice.device,
                                             &descriptorPoolCreateInfo,
                                             nullptr,
                                             &mDescriptorPool);
    vulkanCheck(result, "Failed to create descriptor pool.");
}
