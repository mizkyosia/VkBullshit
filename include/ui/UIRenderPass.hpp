#pragma once
#include "global.hpp"

#include <RenderPass.hpp>

class UIRenderPass : public RenderPass
{
private:
    void createRenderPass() override;

public:
    UIRenderPass(const Device &device, const SwapChain &swapChain);
};
