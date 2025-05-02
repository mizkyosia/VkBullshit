#pragma once
#include "global.hpp"

#include <vector>
#include <functional>

// Forward declaration
class Device;
class RenderPass;
class SwapChain;
class GraphicsPipeline;

class CommandPool
{
private:
    std::vector<VkCommandBuffer> _commandBuffers;

    VkCommandPool _pool;
    VkCommandPoolCreateFlags _flags;

    const Device &_device;
    const RenderPass &_renderPass;
    const SwapChain &_swapChain;
    const GraphicsPipeline &_graphicsPipeline;

    virtual void createCommandBuffers();
    void destroyCommandBuffers();

public:
    CommandPool(const Device &device, const RenderPass &renderPass, const SwapChain &swapChain, const GraphicsPipeline &graphicsPipeline, const VkCommandPoolCreateFlags &flags);
    ~CommandPool();

    virtual void recreateCommandBuffers();
    virtual void recordCommandBuffer(uint32_t index);

    // Getters
    inline const VkCommandPool &pool() const { return _pool; };
    inline VkCommandBuffer &command(uint32_t index) { return _commandBuffers[index]; }
    inline const VkCommandBuffer &command(uint32_t index) const { return _commandBuffers[index]; }
};