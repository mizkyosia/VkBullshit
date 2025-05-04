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

void CommandPool::SingleTimeCommands(const Device &device, VkCommandPool &pool, const std::function<void(const VkCommandBuffer &)> &func)
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer = {};
    if (vkAllocateCommandBuffers(device.logical(), &allocInfo, &commandBuffer) !=
        VK_SUCCESS)
        throw std::runtime_error("failed to allocate command buffers!");

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("Could not create one-time command buffer!");

    func(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        throw std::runtime_error("failed to record command buffer!");

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(device.graphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(device.graphicsQueue());

    vkFreeCommandBuffers(device.logical(), pool, 1, &commandBuffer);
}
