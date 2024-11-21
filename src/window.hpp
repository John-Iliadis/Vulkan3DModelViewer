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
#include "model/model.hpp"


class Window
{
public:
    Window();
    ~Window();

    void run();

private:
    void initializeGLFW();
    void renderFrame();

    void createDepthBuffer();
    void createRenderPass();
    void createFramebuffers();
    void createViewProjUBO();
    void createDescriptorPool();
    void createDescriptorSets();
    void destroyDescriptorResources();

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
    GLFWwindow* mWindow;
    VulkanInstance mInstance;
    VulkanRenderDevice mRenderDevice;

    VkRenderPass mRenderPass;

    VulkanImage mDepthImage;
    std::vector<VkFramebuffer> mSwapchainFramebuffers;

    VulkanBuffer mViewProjUBO;

    VkDescriptorPool mDescriptorPool;
    VkDescriptorSetLayout mLayout0;
    VkDescriptorSet mSet0;

    Model mModel;
};


#endif //VULKAN3DMODELVIEWER_WINDOW_HPP
