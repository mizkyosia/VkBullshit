#include <GraphicsPipeline.hpp>

#include <fstream>
#include <cstring>

#include <Device.hpp>
#include <SwapChain.hpp>
#include <RenderPass.hpp>

GraphicsPipeline::GraphicsPipeline(const Device &device, const SwapChain &swapChain, const RenderPass &renderPass, const std::vector<ShaderInfo> shaders) : _device(device),
                                                                                                                                                            _swapChain(swapChain),
                                                                                                                                                            _renderPass(renderPass),
                                                                                                                                                            _shaders(shaders),
                                                                                                                                                            _oldLayout(VK_NULL_HANDLE)
{
    createPipeline();
}

GraphicsPipeline::~GraphicsPipeline()
{
    vkDestroyPipeline(_device.logical(), _pipeline, nullptr);
    vkDestroyPipelineLayout(_device.logical(), _layout, nullptr);
}

void GraphicsPipeline::recreate()
{
    
}

void GraphicsPipeline::createPipeline()
{
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    shaderStages.reserve(_shaders.size());

    for (auto &shaderInfo : _shaders)
    {
        auto shader = createShaderModule(shaderInfo);

        // Create shader stage for vertex shader
        VkPipelineShaderStageCreateInfo shaderStageInfo{};
        shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfo.stage = (shaderInfo.fragmentShader ? VK_SHADER_STAGE_FRAGMENT_BIT : VK_SHADER_STAGE_VERTEX_BIT);
        shaderStageInfo.module = shader;
        shaderStageInfo.pName = "main";

        shaderStages.push_back(shaderStageInfo);
    }

    // Dynamic pipeline state
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // No vertex data to load at the moment (will come back later)
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    // We're using a triangle list here, but we can change it later if needed
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Create viewport from (0, 0) to (w, h)
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)_swapChain.extent().width;
    viewport.height = (float)_swapChain.extent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Scissor rect for cutting the viewport
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = _swapChain.extent();

    // Define viewport state
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

#pragma region Rasterization
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    // If set to VK_TRUE, discards all fragments (shows nothing to screen)
    rasterizer.rasterizerDiscardEnable = VK_FALSE;

    // How to deal with polygons :
    // - FILL : fills polygons with fragments
    // - LINE : only draws polygon edges as lines
    // - POINT : draws polygon vertices as points
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

    // Line width in fragments (pixels)
    // If that were to be > 1, needs to enable the `wideLines` GPU feature
    rasterizer.lineWidth = 1.0f;

    // Specifies the faces to actually render & their order
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

    // Not using this atm
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f;          // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f;    // Optional

#pragma endregion

#pragma region Multisampling
    // We're not using multisampling rn, so we'll pass a simple default struct
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;          // Optional
    multisampling.pSampleMask = nullptr;            // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE;      // Optionals
#pragma endregion

#pragma region Color Blending

    // Color blend attachment !!!! for 1 framebuffer only !!!!!
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    // Curently using alpha blending mode
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    // Global color blend settings
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    // Disabled bitwise blending mode
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

#pragma endregion

    // Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;            // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr;         // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0;    // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(_device.logical(), &pipelineLayoutInfo, nullptr, &_layout) != VK_SUCCESS)
        throw std::runtime_error("failed to create pipeline layout!");

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages.data();
    // Pass all parameters built before
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    // Pass pipeline layout as well
    pipelineInfo.layout = _layout;
    // Integrate render pass, using subpass with id=0 for render
    pipelineInfo.renderPass = _renderPass.handle();
    pipelineInfo.subpass = 0;
    // May be useful for derived pipelines, instead of re-creating
    // everything for a slightly different pipeline.
    // Optional.
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    // FINALLY, we create the actual pipeline !
    if (vkCreateGraphicsPipelines(_device.logical(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_pipeline) != VK_SUCCESS)
        throw std::runtime_error("failed to create graphics pipeline!");

    // Destroy shader objects, no longer needed
    for (auto &shader : shaderStages)
        vkDestroyShaderModule(_device.logical(), shader.module, nullptr);
}

VkShaderModule GraphicsPipeline::createShaderModule(const ShaderInfo &shader)
{
    auto fullShaderName = (shader.fragmentShader ? "frag/" : "vert/") + shader.name + ".spv";
    std::ifstream file(SHADERS_PATH + fullShaderName, std::ios::ate | std::ios::binary);

    if (!file.is_open())
        throw std::runtime_error("Failed to open shader file!");

    // Read the shader data entirely
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> code(fileSize);
    file.seekg(0);
    file.read(code.data(), fileSize);

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    // This cast could be problematic since we cast from char to u32, luckily std::vector takes the worst alignment case
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    // Create the actual module
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(_device.logical(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

ShaderInfo::ShaderInfo(std::string name, bool fragmentShader) : name(name), fragmentShader(fragmentShader)
{
}
