//
// Created by Gianni on 17/11/2024.
//

#ifndef VULKAN3DMODELVIEWER_APPLICATION_HPP
#define VULKAN3DMODELVIEWER_APPLICATION_HPP

#include <array>
#include <vulkan/vulkan.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include "vk/vulkan_types.hpp"
#include "vk/vulkan_functions.hpp"
#include "model/model.hpp"
#include "camera/camera.hpp"


class Application
{
public:
    Application();
    ~Application();

    void run();

private:
    void initializeGLFW();
    void createDepthBuffer();
    void createRenderPass();
    void createFramebuffers();
    void createViewProjUBO();
    void updateViewProjUBO();
    void createDescriptorPool();
    void createDescriptorSetLayouts();
    void createDescriptorSets();
    void createDescriptorResources();
    void destroyDescriptorResources();
    void createPipelineLayout();
    void createGraphicsPipeline();
    void setupCamera();
    void resize();

    void recordRenderCommands(uint32_t imageIndex);
    void renderFrame();

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPositionCallback(GLFWwindow* window, double x, double y);
    static void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);

private:
    GLFWwindow* mWindow;
    VulkanInstance mInstance;
    VulkanRenderDevice mRenderDevice;

    VkRenderPass mRenderPass;
    VkPipelineLayout mPipelineLayout;
    VkPipeline mGraphicsPipeline;

    VulkanImage mDepthImage;
    std::vector<VkFramebuffer> mFramebuffers;

    VulkanBuffer mModelViewProjUBO;
    VulkanBuffer mMaterialsUBO;

    VkDescriptorPool mDescriptorPool;
    VkDescriptorSetLayout mLayout0;
    VkDescriptorSetLayout mLayout1;
    VkDescriptorSet mSet0;
    VkDescriptorSet mSet1;

    glm::mat4 mModelMatrix;
    Camera mCamera;
    Model mModel;

    bool mLeftMouseButtonPressed;
    double mCursorPosX;
    double mCursorPosY;
    float mRotationX;
    float mRotationY;
    float mScale;
    float mOrbitNavSensitivity;
};


#endif //VULKAN3DMODELVIEWER_APPLICATION_HPP
