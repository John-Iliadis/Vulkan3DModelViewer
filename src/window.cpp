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
    createDescriptorSets();
}

Window::~Window()
{
    destroyDescriptorResources();
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

void Window::createDescriptorSets()
{
    VkDescriptorSetLayoutBinding layout0Binding0 {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
    };

    VkDescriptorSetLayoutCreateInfo layout0CreateInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &layout0Binding0
    };

    VkDescriptorSetLayoutBinding layout1Binding0 {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };

    VkDescriptorSetLayoutBinding layout1Binding1 {
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };

    std::vector<VkDescriptorSetLayoutBinding> layout1Bindings {layout1Binding0, layout1Binding1};

    VkDescriptorSetLayoutCreateInfo layout1CreateInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 2,
        .pBindings = layout1Bindings.data()
    };

    VkResult result = vkCreateDescriptorSetLayout(mRenderDevice.device, &layout0CreateInfo, nullptr, &mLayout0);
    vulkanCheck(result, "Failed to create layout 0.");
    result = vkCreateDescriptorSetLayout(mRenderDevice.device, &layout1CreateInfo, nullptr, &mLayout1);
    vulkanCheck(result, "Failed to create layout 1.");

    std::vector<VkDescriptorSetLayout> layouts {mLayout0, mLayout1};

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = mDescriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data()
    };

    std::vector<VkDescriptorSet> sets {mSet0, mSet1};

    vkAllocateDescriptorSets(mRenderDevice.device, &descriptorSetAllocateInfo, sets.data());
}

void Window::destroyDescriptorResources()
{
    vkDestroyDescriptorSetLayout(mRenderDevice.device, mLayout0, nullptr);
    vkDestroyDescriptorSetLayout(mRenderDevice.device, mLayout1, nullptr);
    vkDestroyDescriptorPool(mRenderDevice.device, mDescriptorPool, nullptr);
}
