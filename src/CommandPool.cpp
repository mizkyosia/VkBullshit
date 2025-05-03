#include <CommandPool.hpp>
#include <SwapChain.hpp>
#include <Device.hpp>
#include <RenderPass.hpp>
#include <GraphicsPipeline.hpp>
#include <QueueFamily.hpp>

CommandPool::CommandPool(const Device &device,
                         const RenderPass &renderPass,
                         const SwapChain &swapChain,
                         const GraphicsPipeline &graphicsPipeline,
                         const VkCommandPoolCreateFlags &flags) : _flags(flags),
                                                                  _device(device),
                                                                  _renderPass(renderPass),
                                                                  _swapChain(swapChain),
                                                                  _graphicsPipeline(graphicsPipeline)
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = _device.queueFamilyIndices().graphicsFamily.value();

    if (vkCreateCommandPool(_device.logical(), &poolInfo, nullptr, &_pool) != VK_SUCCESS)
        throw std::runtime_error("failed to create command pool!");

    createCommandBuffers();
}

CommandPool::~CommandPool()
{
    destroyCommandBuffers();
    vkDestroyCommandPool(_device.logical(), _pool, nullptr);
}

void CommandPool::recreateCommandBuffers()
{
    destroyCommandBuffers();
    createCommandBuffers();
}

void CommandPool::recordCommandBuffer(uint32_t index)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;                  // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(_commandBuffers[index], &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("failed to begin recording command buffer!");

    // Begin the render pass
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _renderPass.handle();
    renderPassInfo.framebuffer = _renderPass.frameBuffer(index);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = _swapChain.extent();

    // Choose the color to clear the image : solid black for now
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    // ------------- BEGINNING RENDER PASS ------------------
    // No secondary buffer commands, so INLINE it is
    vkCmdBeginRenderPass(_commandBuffers[index], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(_commandBuffers[index], VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline.pipeline());

    // We specified use of dynamic viewport & scissor states, so we have to configure them before drawing
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(_swapChain.extent().width);
    viewport.height = static_cast<float>(_swapChain.extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(_commandBuffers[index], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = _swapChain.extent();
    vkCmdSetScissor(_commandBuffers[index], 0, 1, &scissor);

    // LETSGOOOO WE'RE DRAWING NOW !!!!!!
    // A bit underwhelming, yeah, but it'll change later
    vkCmdDraw(_commandBuffers[index], 3, 1, 0, 0);

    // End render pass
    vkCmdEndRenderPass(_commandBuffers[index]);

    // Close command buffer
    if (vkEndCommandBuffer(_commandBuffers[index]) != VK_SUCCESS)
        throw std::runtime_error("failed to record command buffer!");
}

void CommandPool::createCommandBuffers()
{
    // Allow only as many commandbffers as there are frames in the render pass
    _commandBuffers.resize(_renderPass.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = _pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = _commandBuffers.size();

    if (vkAllocateCommandBuffers(_device.logical(), &allocInfo, _commandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate command buffers!");
}

void CommandPool::destroyCommandBuffers()
{
    vkFreeCommandBuffers(_device.logical(), _pool, _commandBuffers.size(), _commandBuffers.data());
}
