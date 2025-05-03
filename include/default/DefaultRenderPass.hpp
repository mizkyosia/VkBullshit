#pragma once
#include "global.hpp"

#include <RenderPass.hpp>

class DefaultRenderPass : public RenderPass
{
private:
    void createRenderPass() override;

public:
    DefaultRenderPass(const Device &device, const SwapChain &swapChain);
};