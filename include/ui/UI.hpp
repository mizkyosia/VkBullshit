#pragma once
#include "global.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <Window.hpp>
#include <Device.hpp>
#include <SwapChain.hpp>
#include <GraphicsPipeline.hpp>

#include <ui/UIRenderPass.hpp>
#include <ui/UICommandPool.hpp>

class UI
{
private:
    const Window &_window;
    const Device &_device;
    const SwapChain &_swapChain;
    const GraphicsPipeline &_graphicsPipeline;

    UIRenderPass _renderPass;
    UICommandPool _commandPool;
    VkDescriptorPool _imGuiDescriptorPool;

    void createImGuiDescriptorPool();

public:
    UI(const Window &window, const Device &device, const SwapChain &swapChain, const GraphicsPipeline &graphicsPipeline);
    ~UI();

    inline VkCommandBuffer &command(uint32_t index) { return _commandPool.command(index); }
    inline const VkCommandBuffer &command(uint32_t index) const { return _commandPool.command(index); }
    void recordCommandBuffers(uint32_t bufferIdx) { _commandPool.recordCommandBuffer(bufferIdx); }

    void recreate();

    void draw();
};