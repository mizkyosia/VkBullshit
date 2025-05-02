#pragma once
#include "global.hpp"

#include <vector>

// Forward declaration
class Device;
class SwapChain;

class RenderPass
{
protected:
    VkRenderPass _renderPass;
    VkRenderPass _oldRenderPass;

    std::vector<VkFramebuffer> _frameBuffers;

    const Device &_device;
    const SwapChain &_swapChain;

    virtual void createRenderPass();
    void createFrameBuffers();

    void destroyFrameBuffers();

public:
    RenderPass(const Device &device, const SwapChain &swapChain);
    ~RenderPass();

    void recreate();
    void cleanupOld();

    // Getters
    inline const VkRenderPass &handle() const { return _renderPass; }
    inline const VkFramebuffer &frameBuffer(uint32_t index) const { return _frameBuffers[index]; }
    inline size_t size() const { return _frameBuffers.size(); }
};