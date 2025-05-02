#include <RenderPass.hpp>
#include <Device.hpp>
#include <SwapChain.hpp>

void RenderPass::createRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = _swapChain.imageFormat();
    // No multisampling, so 1 sample
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    // Clear framebufer before drawing on it
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // Store contents in framebuffer after the render
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    // Not using stencils, don't care
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // We don't care about the previous layout, so we can drop all previous data in the buffer with LAYOUT_UNDEFINED
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // Images are to be presented in a swapchain ; we use the PRESENT_SRC preset then
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    // Only 1 colorAttachmentDescrption, so the chosen index is 0
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Create subpass
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    // Only 1 color attacment
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    // Dependencies for the subpass
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    // Wait for swapchain to finish reading the image
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    // Set the waiting operations
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // Finally, create actual render pass
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    // Add dependencies
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(_device.logical(), &renderPassInfo, nullptr, &_renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}

void RenderPass::createFrameBuffers()
{
    auto nbImageViews = _swapChain.numImageViews();
    _frameBuffers.resize(nbImageViews);

    for (size_t i = 0; i < nbImageViews; i++)
    {
        VkImageView attachments[] = {_swapChain.imageView(i)};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = _renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = _swapChain.extent().width;
        framebufferInfo.height = _swapChain.extent().height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(_device.logical(), &framebufferInfo, nullptr, &_frameBuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create framebuffer!");
    }
}

void RenderPass::destroyFrameBuffers()
{
    for (auto &buffer : _frameBuffers)
        vkDestroyFramebuffer(_device.logical(), buffer, nullptr);
}

RenderPass::RenderPass(const Device &device, const SwapChain &swapChain) : _oldRenderPass(VK_NULL_HANDLE),
                                                                           _device(device),
                                                                           _swapChain(swapChain)
{
    createRenderPass();
    createFrameBuffers();
}

RenderPass::~RenderPass()
{
    destroyFrameBuffers();
    cleanupOld();
    vkDestroyRenderPass(_device.logical(), _renderPass, nullptr);
}

void RenderPass::cleanupOld()
{
    if (_oldRenderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(_device.logical(), _oldRenderPass, nullptr);
        _oldRenderPass = VK_NULL_HANDLE;
    }
}

void RenderPass::recreate()
{
    destroyFrameBuffers();
    cleanupOld();
    _oldRenderPass = _renderPass;
    createRenderPass();
    createFrameBuffers();
}