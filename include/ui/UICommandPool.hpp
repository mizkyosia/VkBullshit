#pragma once
#include "global.hpp"

#include <Renderer.hpp>

class UICommandPool : public Renderer
{
private:
    void createCommandBuffers() override;

public:
    UICommandPool(const Device &device, const RenderPass &renderPass, const SwapChain &swapChain, const GraphicsPipeline &graphicsPipeline, const VkCommandPoolCreateFlags &flags);

    void recordCommandBuffer(uint32_t index) override;
};
