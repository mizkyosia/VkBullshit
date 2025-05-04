#pragma once
#include "global.hpp"

#include <Renderer.hpp>

#include <geometry/Vertex.hpp>

class BaseRenderer : public Renderer
{
private:
    void createCommandBuffers() override;

    VkBuffer _vertexBuffer;
    VkDeviceMemory _vertexBufferMemory;

    void createVertexBuffer();

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

public:
    std::vector<Vertex> vertices;

    BaseRenderer(const Device &device, const RenderPass &renderPass, const SwapChain &swapChain, const GraphicsPipeline &graphicsPipeline, const VkCommandPoolCreateFlags &flags, std::vector<Vertex> vertices);
    ~BaseRenderer();

    void recordCommandBuffer(uint32_t index) override;
};
