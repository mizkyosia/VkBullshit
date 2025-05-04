#pragma once
#include "global.hpp"

#include <CommandPool.hpp>

class DefaultCommandPool : public CommandPool
{
private:
    void createCommandBuffers() override;

public:
    DefaultCommandPool(const Device &device, const RenderPass &renderPass, const SwapChain &swapChain, const GraphicsPipeline &graphicsPipeline, const VkCommandPoolCreateFlags &flags);

    void recordCommandBuffer(uint32_t index) override;
};
