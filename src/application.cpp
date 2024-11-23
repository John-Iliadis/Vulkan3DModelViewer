//
// Created by Gianni on 17/11/2024.
//

#include "application.hpp"


static constexpr int INITIAL_WINDOW_WIDTH = 1920;
static constexpr int INITIAL_WINDOW_HEIGHT = 1080;
static constexpr char* const WINDOW_TITLE = "3D Model Viewer";

Application::Application()
    : mLeftMouseButtonPressed()
    , mCursorPosX()
    , mCursorPosY()
    , mRotationX()
    , mRotationY()
    , mScale(1.f)
    , mOrbitNavSensitivity(0.15f)
{
    initializeGLFW();
    createInstance(mInstance);
    createSurface(mInstance, mWindow);
    createRenderingDevice(mInstance, mRenderDevice);
    createDepthBuffer();
    createViewProjUBO();

    setupCamera();
    updateViewProjUBO();
    createModel(mModel, mRenderDevice, "../assets/sponza/sponza.obj");

    createDescriptorResources();
    createRenderPass();
    createFramebuffers();
    createGraphicsPipeline();
}

Application::~Application()
{
    destroyBuffer(mRenderDevice, mModelViewProjUBO);
    vkDestroyPipeline(mRenderDevice.device, mGraphicsPipeline, nullptr);
    vkDestroyPipelineLayout(mRenderDevice.device, mPipelineLayout, nullptr);
    destroyModel(mModel, mRenderDevice);
    std::for_each(mFramebuffers.begin(), mFramebuffers.end(),
                  [this] (auto fb) { vkDestroyFramebuffer(mRenderDevice.device, fb,nullptr); });
    vkDestroyRenderPass(mRenderDevice.device, mRenderPass, nullptr);
    destroyImage(mRenderDevice, mDepthImage);
    destroyDescriptorResources();
    destroyRenderingDevice(mRenderDevice);
    destroyInstance(mInstance);
    glfwTerminate();
}

void Application::run()
{
    while (!glfwWindowShouldClose(mWindow))
    {
        glfwPollEvents();
        renderFrame();
    }

     vkDeviceWaitIdle(mRenderDevice.device);
}

void Application::initializeGLFW()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    mWindow = glfwCreateWindow(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
    vulkanCheck(mWindow? VK_SUCCESS : static_cast<VkResult>(~VK_SUCCESS), "Failed to create window");

    glfwSetWindowUserPointer(mWindow, this);
    glfwSetKeyCallback(mWindow, keyCallback);
    glfwSetMouseButtonCallback(mWindow, mouseButtonCallback);
    glfwSetCursorPosCallback(mWindow, cursorPositionCallback);
    glfwSetScrollCallback(mWindow, scrollCallback);
}

void Application::createDepthBuffer()
{
    mDepthImage = createImage(mRenderDevice,
                              VK_FORMAT_D32_SFLOAT,
                              mRenderDevice.swapchainExtent.width,
                              mRenderDevice.swapchainExtent.height,
                              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                              VK_IMAGE_ASPECT_DEPTH_BIT);
}

void Application::createRenderPass()
{
    VkAttachmentDescription colorAttachment {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentDescription depthAttachment {
        .format = VK_FORMAT_D32_SFLOAT,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    std::vector<VkAttachmentDescription> attachments {colorAttachment, depthAttachment};

    VkAttachmentReference colorAttachmentRef {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkAttachmentReference depthAttachmentRef {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
        .pDepthStencilAttachment = &depthAttachmentRef
    };

    VkRenderPassCreateInfo renderPassCreateInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass
    };

    VkResult result = vkCreateRenderPass(mRenderDevice.device, &renderPassCreateInfo, nullptr, &mRenderPass);
    vulkanCheck(result, "Failed to create renderpass.");
}

void Application::createFramebuffers()
{
    size_t imageCount = mRenderDevice.swapchainImages.size();
    mFramebuffers.resize(imageCount);

    for (size_t i = 0; i < imageCount; ++i)
    {
        std::vector<VkImageView> attachments {mRenderDevice.swapchainImageViews.at(i), mDepthImage.imageView};

        VkFramebufferCreateInfo framebufferCreateInfo {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = mRenderPass,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .width = mRenderDevice.swapchainExtent.width,
            .height = mRenderDevice.swapchainExtent.height,
            .layers = 1
        };

        VkResult result = vkCreateFramebuffer(mRenderDevice.device, &framebufferCreateInfo, nullptr, &mFramebuffers.at(i));
        vulkanCheck(result, "Failed to create framebuffer.");
    }
}

void Application::createViewProjUBO()
{
    VkBufferUsageFlags usage {
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
        VK_BUFFER_USAGE_TRANSFER_DST_BIT
    };

    VkMemoryPropertyFlags memoryProperties  = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    mModelViewProjUBO = createBuffer(mRenderDevice,
                                     sizeof(glm::mat4),
                                     usage,
                                     memoryProperties);
}

void Application::updateViewProjUBO()
{
    static constexpr glm::mat4 identity(1.f);

    mModelMatrix = glm::rotate(identity, glm::radians(mRotationY), {1.f, 0.f, 0.f});
    mModelMatrix = glm::rotate(mModelMatrix, glm::radians(mRotationX), {0.f, 1.f, 0.f});
    mModelMatrix = glm::scale(mModelMatrix, glm::vec3(mScale));

    glm::mat4 mvp = mCamera.viewProjection() * mModelMatrix;

    VkCommandBuffer commandBuffer = beginSingleCommand(mRenderDevice);
    vkCmdUpdateBuffer(commandBuffer, mModelViewProjUBO.buffer, 0, sizeof(glm::mat4), &mvp);
    endSingleCommand(mRenderDevice, commandBuffer);
}

void Application::createDescriptorPool()
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(mRenderDevice.physicalDevice, &physicalDeviceProperties);

    uint32_t maxPerStageDescriptorSamplers = physicalDeviceProperties.limits.maxPerStageDescriptorSamplers;

    std::vector<VkDescriptorPoolSize> descriptorPoolSizes {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxPerStageDescriptorSamplers}
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

void Application::createDescriptorSetLayouts()
{
    VkDescriptorSetLayoutBinding layout0Binding1 {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
    };

    VkDescriptorSetLayoutCreateInfo layout0CreateInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &layout0Binding1
    };

    VkResult result = vkCreateDescriptorSetLayout(mRenderDevice.device, &layout0CreateInfo, nullptr, &mLayout0);
    vulkanCheck(result, "Failed to create descriptor set layout 0.");

    VkDescriptorSetLayoutBinding layout1Binding0 {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };

    VkDescriptorSetLayoutBinding layout1Binding1 {
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = static_cast<uint32_t>(mModel.textures.size()),
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };

    std::array<VkDescriptorSetLayoutBinding, 2> layout1Bindings {
        layout1Binding0,
        layout1Binding1
    };

    VkDescriptorSetLayoutCreateInfo layout1CreateInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = static_cast<uint32_t>(layout1Bindings.size()),
        .pBindings = layout1Bindings.data()
    };

    result = vkCreateDescriptorSetLayout(mRenderDevice.device, &layout1CreateInfo, nullptr, &mLayout1);
    vulkanCheck(result, "Failed to create descriptor set layout 1.");
}

void Application::createDescriptorSets()
{
    std::array<VkDescriptorSetLayout, 2> layouts {
        mLayout0,
        mLayout1
    };

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = mDescriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data()
    };

    VkDescriptorSet* sets[] {&mSet0, &mSet1};

    VkResult result = vkAllocateDescriptorSets(mRenderDevice.device, &descriptorSetAllocateInfo, sets[0]);
    vulkanCheck(result, "Failed to allocate descriptor sets.");

    // update set 0
    std::array<VkWriteDescriptorSet, 3> descriptorWrites;

    VkDescriptorBufferInfo mvpBufferInfo {
        .buffer = mModelViewProjUBO.buffer,
        .offset = 0,
        .range = VK_WHOLE_SIZE
    };

    descriptorWrites.at(0) = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = mSet0,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &mvpBufferInfo
    };

    // update set 1
    VkDescriptorBufferInfo materialBufferInfo {
        .buffer = mModel.materialBuffer.buffer,
        .offset = 0,
        .range = VK_WHOLE_SIZE
    };

    descriptorWrites.at(1) = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = mSet1,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .pBufferInfo = &materialBufferInfo
    };

    uint32_t textureCount = mModel.textures.size();
    std::vector<VkDescriptorImageInfo> texturesInfo(textureCount);
    for (uint32_t i = 0; i < textureCount; ++i)
    {
        texturesInfo.at(i) = {
            .sampler = mModel.textures.at(i).sampler,
            .imageView = mModel.textures.at(i).image.imageView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
    }

    descriptorWrites.at(2) = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = mSet1,
        .dstBinding = 1,
        .dstArrayElement = 0,
        .descriptorCount = textureCount,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = texturesInfo.data()
    };

    vkUpdateDescriptorSets(mRenderDevice.device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

void Application::createDescriptorResources()
{
    createDescriptorPool();
    createDescriptorSetLayouts();
    createDescriptorSets();
}

void Application::destroyDescriptorResources()
{
    vkDestroyDescriptorSetLayout(mRenderDevice.device, mLayout0, nullptr);
    vkDestroyDescriptorSetLayout(mRenderDevice.device, mLayout1, nullptr);
    vkDestroyDescriptorPool(mRenderDevice.device, mDescriptorPool, nullptr);
}

void Application::createPipelineLayout()
{
    std::array<VkDescriptorSetLayout, 2> layouts {
        mLayout0,
        mLayout1
    };

    VkPushConstantRange pushConstantRange {
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset = 0,
        .size = sizeof(uint32_t)
    };

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = static_cast<uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data(),
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstantRange
    };

    VkResult result = vkCreatePipelineLayout(mRenderDevice.device,
                                             &pipelineLayoutCreateInfo,
                                             nullptr,
                                             &mPipelineLayout);
    vulkanCheck(result, "Failed to create the pipeline layout.");
}

void Application::createGraphicsPipeline()
{
    createPipelineLayout();

    VkShaderModule vertexShader = createShaderModule(mRenderDevice, "shaders/model_vert.spv");
    VkShaderModule fragmentShader = createShaderModule(mRenderDevice, "shaders/model_frag.spv");

    VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertexShader,
        .pName = "main"
    };

    VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragmentShader,
        .pName = "main"
    };

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages {
        vertexShaderStageCreateInfo,
        fragmentShaderStageCreateInfo
    };

    auto bindingDescription = Vertex::bindingDescription();
    auto attributeDescription = Vertex::attributeDescription();
    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size()),
        .pVertexAttributeDescriptions = attributeDescription.data()
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
    };

    VkPipelineTessellationStateCreateInfo tessellationStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO
    };

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1
    };

    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_NONE,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1.f
    };

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_16_BIT,
        .sampleShadingEnable = VK_FALSE
    };

    VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState {
        .blendEnable = VK_FALSE,
        .colorWriteMask {
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT
        }
    };

    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_CLEAR,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachmentState
    };

    std::vector<VkDynamicState> dynamicStates {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()
    };

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = static_cast<uint32_t>(shaderStages.size()),
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertexInputStateCreateInfo,
        .pInputAssemblyState = &inputAssemblyStateCreateInfo,
        .pTessellationState = &tessellationStateCreateInfo,
        .pViewportState = &viewportStateCreateInfo,
        .pRasterizationState = &rasterizationStateCreateInfo,
        .pMultisampleState = &multisampleStateCreateInfo,
        .pDepthStencilState = &depthStencilStateCreateInfo,
        .pColorBlendState = &colorBlendStateCreateInfo,
        .pDynamicState = &dynamicStateCreateInfo,
        .layout = mPipelineLayout,
        .renderPass = mRenderPass,
        .subpass = 0
    };

    VkResult result = vkCreateGraphicsPipelines(mRenderDevice.device,
                                       VK_NULL_HANDLE,
                                       1, &graphicsPipelineCreateInfo,
                                       nullptr,
                                       &mGraphicsPipeline);
    vulkanCheck(result, "Failed to create graphics pipeline.");

    vkDestroyShaderModule(mRenderDevice.device, vertexShader, nullptr);
    vkDestroyShaderModule(mRenderDevice.device, fragmentShader, nullptr);
}

void Application::setupCamera()
{
    mCamera = Camera(45.f, INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT);
    mCamera.setPosition(0, 0, 5);
}

void Application::recordRenderCommands(uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    vkBeginCommandBuffer(mRenderDevice.commandBuffer, &beginInfo);

    static std::vector<VkClearValue> clearValues {
        {.color = {0.2f, 0.2f, 0.2f, 1.f}},
        {.depthStencil = {1.f, 0}}
    };

    VkRenderPassBeginInfo renderPassBeginInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = mRenderPass,
        .framebuffer = mFramebuffers.at(imageIndex),
        .renderArea {
            .offset = {0, 0},
            .extent = mRenderDevice.swapchainExtent
        },
        .clearValueCount = static_cast<uint32_t>(clearValues.size()),
        .pClearValues = clearValues.data()
    };

    VkViewport viewport {
        .x = 0,
        .y = 0,
        .width = static_cast<float>(mRenderDevice.swapchainExtent.width),
        .height = static_cast<float>(mRenderDevice.swapchainExtent.height),
        .minDepth = 0.f,
        .maxDepth = 1.f
    };

    VkRect2D scissor {
        .offset = {0, 0},
        .extent = mRenderDevice.swapchainExtent
    };

    vkCmdBeginRenderPass(mRenderDevice.commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(mRenderDevice.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);
    vkCmdSetViewport(mRenderDevice.commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(mRenderDevice.commandBuffer, 0, 1, &scissor);

    std::array<VkDescriptorSet, 2> descriptorSets {mSet0, mSet1};
    vkCmdBindDescriptorSets(mRenderDevice.commandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            mPipelineLayout,
                            0, descriptorSets.size(), descriptorSets.data(),
                            0, nullptr);

    renderModel(mModel, mRenderDevice, mPipelineLayout, mRenderDevice.commandBuffer);
    vkCmdEndRenderPass(mRenderDevice.commandBuffer);

    vkEndCommandBuffer(mRenderDevice.commandBuffer);
}

void Application::renderFrame()
{
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(mRenderDevice.device,
                                            mRenderDevice.swapchain,
                                            UINT64_MAX,
                                            mRenderDevice.imageReadySemaphore,
                                            VK_NULL_HANDLE,
                                            &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        resize();
        return;
    }

    recordRenderCommands(imageIndex);

    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

    VkSubmitInfo renderSubmitInfo {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &mRenderDevice.imageReadySemaphore,
        .pWaitDstStageMask = &dstStage,
        .commandBufferCount = 1,
        .pCommandBuffers = &mRenderDevice.commandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &mRenderDevice.renderFinishedSemaphore
    };

    vkQueueSubmit(mRenderDevice.graphicsQueue, 1, &renderSubmitInfo, VK_NULL_HANDLE);

    VkPresentInfoKHR presentInfo {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &mRenderDevice.renderFinishedSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &mRenderDevice.swapchain,
        .pImageIndices = &imageIndex
    };

    result = vkQueuePresentKHR(mRenderDevice.graphicsQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        resize();

    vkDeviceWaitIdle(mRenderDevice.device);
}

void Application::resize()
{
    int width, height;
    glfwGetFramebufferSize(mWindow, &width, &height);

    while (width == 0 || height == 0)
    {
        glfwWaitEvents();
        glfwGetFramebufferSize(mWindow, &width, &height);
    }

    vkDeviceWaitIdle(mRenderDevice.device);

    // destroy resources
    for (size_t i = 0, size = mRenderDevice.swapchainImages.size(); i < size; ++i)
    {
        vkDestroyFramebuffer(mRenderDevice.device, mFramebuffers.at(i), nullptr);
        vkDestroyImageView(mRenderDevice.device, mRenderDevice.swapchainImageViews.at(i), nullptr);
    }

    destroyImage(mRenderDevice, mDepthImage);
    vkDestroySwapchainKHR(mRenderDevice.device, mRenderDevice.swapchain, nullptr);

    // recreate resources
    createSwapchain(mInstance, mRenderDevice);
    createSwapchainImages(mRenderDevice);
    createDepthBuffer();
    createFramebuffers();

    // update resources
    mCamera.resize(mRenderDevice.swapchainExtent.width, mRenderDevice.swapchainExtent.height);
    updateViewProjUBO();

#ifdef DEBUG_MODE
    std::cout << "Resized: " << mRenderDevice.swapchainExtent.width << ' ' << mRenderDevice.swapchainExtent.height << '\n';
#endif
}

void Application::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    Application& app = *reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));

    if (key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void Application::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    Application& app = *reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));

    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            app.mLeftMouseButtonPressed = true;
        }
        else if (action == GLFW_RELEASE)
        {
            app.mLeftMouseButtonPressed = false;
        }
    }
}

void Application::cursorPositionCallback(GLFWwindow *window, double x, double y)
{
    Application& app = *reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));

    if (app.mLeftMouseButtonPressed)
    {
        double dx = x - app.mCursorPosX;
        double dy = y - app.mCursorPosY;

        app.mRotationX += dx * app.mOrbitNavSensitivity;
        app.mRotationY += dy * app.mOrbitNavSensitivity;

        if (glm::abs(app.mRotationY) > 90.f)
        {
            app.mRotationY = glm::clamp(app.mRotationY, -90.f, 90.f);
        }

        app.updateViewProjUBO();
    }

    app.mCursorPosX = x;
    app.mCursorPosY = y;
}

void Application::scrollCallback(GLFWwindow *window, double xOffset, double yOffset)
{
    if (yOffset == 0)
        return;

    Application& app = *reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));

    app.mScale += yOffset * app.mScale / 10.f;
    app.mScale = glm::clamp(app.mScale, 0.1f, 10.f);

    app.updateViewProjUBO();
}
