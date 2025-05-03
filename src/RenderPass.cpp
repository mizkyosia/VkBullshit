#include <RenderPass.hpp>
#include <Device.hpp>
#include <SwapChain.hpp>

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