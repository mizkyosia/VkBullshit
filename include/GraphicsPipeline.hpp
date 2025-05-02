#pragma once
#include "global.hpp"

#include <vector>

struct ShaderInfo
{
    ShaderInfo(std::string name, bool fragmentShader);
    std::string name;
    bool fragmentShader;
};

// Forward declaration
class Device;
class SwapChain;
class RenderPass;

class GraphicsPipeline
{
public:
    GraphicsPipeline(const Device &device, const SwapChain &swapChain, const RenderPass &renderPass, const std::vector<ShaderInfo> shaders);
    ~GraphicsPipeline();

    void recreate();

    inline const VkPipeline &pipeline() const { return _pipeline; }
    inline const VkPipelineLayout &layout() const { return _layout; }

private:
    VkPipeline _pipeline;
    VkPipelineLayout _layout;
    VkPipelineLayout _oldLayout;

    const Device &_device;
    const SwapChain &_swapChain;
    const RenderPass &_renderPass;

    const std::vector<ShaderInfo> _shaders;

    void createPipeline();
    VkShaderModule createShaderModule(const ShaderInfo &shader);
};