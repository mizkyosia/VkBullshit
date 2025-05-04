#include <default/BaseRenderer.hpp>

#include <RenderPass.hpp>
#include <Device.hpp>
#include <SwapChain.hpp>
#include <GraphicsPipeline.hpp>

#include <cstring>

BaseRenderer::BaseRenderer(const Device &device, const RenderPass &renderPass, const SwapChain &swapChain, const GraphicsPipeline &graphicsPipeline, const VkCommandPoolCreateFlags &flags, std::vector<Vertex> vertices) : Renderer(device, renderPass, swapChain, graphicsPipeline, flags), vertices(vertices)
{
    createCommandBuffers();
    createVertexBuffer();
}

BaseRenderer::~BaseRenderer()
{
    vkDestroyBuffer(_device.logical(), _vertexBuffer, nullptr);
    vkFreeMemory(_device.logical(), _vertexBufferMemory, nullptr);
}
void BaseRenderer::recordCommandBuffer(uint32_t index)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;                  // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(_commandBuffers[index], &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("failed to begin recording command buffer!");

    // Begin the render pass
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _renderPass.handle();
    renderPassInfo.framebuffer = _renderPass.frameBuffer(index);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = _swapChain.extent();

    // Choose the color to clear the image : solid black for now
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    // ------------- BEGINNING RENDER PASS ------------------
    // No secondary buffer commands, so INLINE it is
    vkCmdBeginRenderPass(_commandBuffers[index], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(_commandBuffers[index], VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline.pipeline());

    // We specified use of dynamic viewport & scissor states, so we have to configure them before drawing
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(_swapChain.extent().width);
    viewport.height = static_cast<float>(_swapChain.extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(_commandBuffers[index], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = _swapChain.extent();
    vkCmdSetScissor(_commandBuffers[index], 0, 1, &scissor);

    // Bind vertex buffers
    VkBuffer vertexBuffers[] = {_vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(_commandBuffers[index], 0, 1, vertexBuffers, offsets);

    // LETSGOOOO WE'RE DRAWING NOW !!!!!!
    // A bit underwhelming, yeah, but it'll change later
    vkCmdDraw(_commandBuffers[index], static_cast<uint32_t>(vertices.size()), 1, 0, 0);

    // End render pass
    vkCmdEndRenderPass(_commandBuffers[index]);

    // Close command buffer
    if (vkEndCommandBuffer(_commandBuffers[index]) != VK_SUCCESS)
        throw std::runtime_error("failed to record command buffer!");
}

void BaseRenderer::createCommandBuffers()
{
    // Allow only as many commandbffers as there are frames in the render pass
    _commandBuffers.resize(_renderPass.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = _pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = _commandBuffers.size();

    if (vkAllocateCommandBuffers(_device.logical(), &allocInfo, _commandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate command buffers!");
}

void BaseRenderer::createVertexBuffer()
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(vertices[0]) * vertices.size();
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(_device.logical(), &bufferInfo, nullptr, &_vertexBuffer) != VK_SUCCESS)
        throw std::runtime_error("failed to create vertex buffer!");

    // Check the memory requirements of the GPU
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(_device.logical(), _vertexBuffer, &memRequirements);

    // Allocate the necessary memory for the vertex buffer
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(_device.logical(), &allocInfo, nullptr, &_vertexBufferMemory) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate vertex buffer memory!");

    // Bind the memory to the buffer
    vkBindBufferMemory(_device.logical(), _vertexBuffer, _vertexBufferMemory, 0);

    // Copy the actual data into the buffer
    void *data;
    vkMapMemory(_device.logical(), _vertexBufferMemory, 0, bufferInfo.size, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferInfo.size);
    vkUnmapMemory(_device.logical(), _vertexBufferMemory);
}

uint32_t BaseRenderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(_device.physical(), &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }

    throw std::runtime_error("failed to find suitable memory type!");
}
