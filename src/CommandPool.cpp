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

void CommandPool::destroyCommandBuffers()
{
    vkFreeCommandBuffers(_device.logical(), _pool, _commandBuffers.size(), _commandBuffers.data());
}

void CommandPool::recordCommandBuffer(uint32_t index) {}
