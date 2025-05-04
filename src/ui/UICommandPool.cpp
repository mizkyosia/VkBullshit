#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <ui/UICommandPool.hpp>

#include <RenderPass.hpp>
#include <Device.hpp>
#include <SwapChain.hpp>
#include <GraphicsPipeline.hpp>

UICommandPool::UICommandPool(const Device &device, const RenderPass &renderPass, const SwapChain &swapChain, const GraphicsPipeline &graphicsPipeline, const VkCommandPoolCreateFlags &flags) : Renderer(device, renderPass, swapChain, graphicsPipeline, flags)
{
    createCommandBuffers();
}

void UICommandPool::recordCommandBuffer(uint32_t index)
{
    VkCommandBufferBeginInfo cmdBufferBegin = {};
    cmdBufferBegin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufferBegin.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(_commandBuffers[index], &cmdBufferBegin) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("Unable to start recording UI command buffer!");
    }

    VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 0.5f};
    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = _renderPass.handle();
    renderPassBeginInfo.framebuffer = _renderPass.frameBuffer(index);
    renderPassBeginInfo.renderArea.extent = _swapChain.extent();
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(_commandBuffers[index], &renderPassBeginInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    // Grab and record the draw data for Dear Imgui
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), _commandBuffers[index]);

    // End and submit render pass
    vkCmdEndRenderPass(_commandBuffers[index]);

    if (vkEndCommandBuffer(_commandBuffers[index]) != VK_SUCCESS)
        throw std::runtime_error("Failed to record command buffers!");
}

void UICommandPool::createCommandBuffers()
{
    _commandBuffers.resize(_renderPass.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = _pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(_commandBuffers.size());

    if (vkAllocateCommandBuffers(_device.logical(), &allocInfo, _commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Unable to allocate UI command buffers!");
    }
}