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
    , mRenderPass()
    , mDepthImage()
    , mViewProjUBO()
    , mDescriptorPool()
    , mLayout0()
{
    initializeGLFW();
    createInstance(mInstance);
    createSurface(mInstance, mWindow);
    createRenderingDevice(mInstance, mRenderDevice);
    createViewProjUBO();
    createDepthBuffer();
    createDescriptorPool();
    createDescriptorSets();
    createRenderPass();
    createFramebuffers();
    createGraphicsPipeline();

    setupCamera();
    updateViewProjUBO();
    createModel(mModel, mRenderDevice, "../assets/backpack/backpack.obj");
}

Window::~Window()
{
    destroyBuffer(mRenderDevice, mViewProjUBO);
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
    glfwSetMouseButtonCallback(mWindow, mouseButtonCallback);
}

void Window::createDepthBuffer()
{
    mDepthImage = createImage(mRenderDevice,
                              VK_FORMAT_D32_SFLOAT,
                              mRenderDevice.swapchainExtent.width,
                              mRenderDevice.swapchainExtent.height,
                              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                              VK_IMAGE_ASPECT_DEPTH_BIT);
}

void Window::createRenderPass()
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

void Window::createFramebuffers()
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

void Window::createViewProjUBO()
{
    mViewProjUBO = createBuffer(mRenderDevice,
                                sizeof(glm::mat4),
                                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

void Window::updateViewProjUBO()
{
    VkCommandBuffer commandBuffer = beginSingleCommand(mRenderDevice);
    vkCmdUpdateBuffer(commandBuffer, mViewProjUBO.buffer, 0, sizeof(glm::mat4), &mCamera.viewProjection());
    endSingleCommand(mRenderDevice, commandBuffer);
}

void Window::createDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> descriptorPoolSizes {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3}
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

    VkResult result = vkCreateDescriptorSetLayout(mRenderDevice.device, &layout0CreateInfo, nullptr, &mLayout0);
    vulkanCheck(result, "Failed to create layout 0.");

    std::vector<VkDescriptorSetLayout> layouts {mLayout0};

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = mDescriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data()
    };

    VkDescriptorSet* sets[] {&mSet0};

    result = vkAllocateDescriptorSets(mRenderDevice.device, &descriptorSetAllocateInfo, sets[0]);
    vulkanCheck(result, "Failed to allocate descriptor set.");

    VkDescriptorBufferInfo uboInfo {
        .buffer = mViewProjUBO.buffer,
        .offset = 0,
        .range = VK_WHOLE_SIZE
    };

    VkWriteDescriptorSet writeDescriptorSet {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = mSet0,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pImageInfo = nullptr,
        .pBufferInfo = &uboInfo
    };

    vkUpdateDescriptorSets(mRenderDevice.device, 1, &writeDescriptorSet, 0, nullptr);
}

void Window::destroyDescriptorResources()
{
    vkDestroyDescriptorSetLayout(mRenderDevice.device, mLayout0, nullptr);
//    vkDestroyDescriptorSetLayout(mRenderDevice.device, mLayout1, nullptr);
    vkDestroyDescriptorPool(mRenderDevice.device, mDescriptorPool, nullptr);
}

void Window::createPipelineLayout()
{
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &mLayout0,
    };

    VkResult result = vkCreatePipelineLayout(mRenderDevice.device,
                                             &pipelineLayoutCreateInfo,
                                             nullptr,
                                             &mPipelineLayout);
    vulkanCheck(result, "Failed to create pipeline layout");
}

void Window::createGraphicsPipeline()
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
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
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

void Window::setupCamera()
{
    mCamera = Camera(45.f, INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT);
    mCamera.setPosition(0, 0, 5);
}

void Window::recordRenderCommands(uint32_t imageIndex)
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
    vkCmdBindDescriptorSets(mRenderDevice.commandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            mPipelineLayout,
                            0, 1, &mSet0,
                            0, nullptr);
    renderModel(mModel, mRenderDevice.commandBuffer);
    vkCmdEndRenderPass(mRenderDevice.commandBuffer);

    vkEndCommandBuffer(mRenderDevice.commandBuffer);
}

void Window::renderFrame()
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

void Window::resize()
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

void Window::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    Window& appWindow = *reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));

    if (key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void Window::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    Window& appWindow = *reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
}
