#include <Application.hpp>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

Application::Application(bool enableValidationLayers) : window("Test", {WIDTH, HEIGHT}, "Vulkan", enableValidationLayers),
                                                        debugMessenger(window),
                                                        device(window),
                                                        swapChain(device, window),
                                                        defaultRenderPass(device, swapChain),
                                                        graphicsPipeline(device, swapChain, defaultRenderPass, {ShaderInfo("base", true), ShaderInfo("base", false)}),
                                                        commandPool(device, defaultRenderPass, swapChain, graphicsPipeline, 0),
                                                        sync(device, swapChain.numImages(), MAX_FRAMES_IN_FLIGHT)
{
}

void Application::drawFrame(bool &resized)
{
    // Wait for the previous frame to be rendered
    vkWaitForFences(device.logical(), 1, &sync.inFlightFence(currentFrame), VK_TRUE, UINT64_MAX);

    // Acquire image for current frame in swapchain. Disables the timeout by putting a very high value
    uint32_t imageIndex;
    vkAcquireNextImageKHR(device.logical(), swapChain.handle(), UINT64_MAX, sync.imageAvailable(currentFrame), VK_NULL_HANDLE, &imageIndex);

    // Reset the current command buffer, so that it may be used again
    vkResetCommandBuffer(commandPool.command(currentFrame), 0);
    // Re-record the command buffer for the current frame/image
    commandPool.recordCommandBuffer(currentFrame);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {sync.imageAvailable(currentFrame)};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    // Set command buffers to give
    VkCommandBuffer buffers[] = {commandPool.command(currentFrame)};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = buffers;

    // Setup waiting semaphores
    VkSemaphore signalSemaphores[] = {sync.renderFinished(currentFrame)};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    // Reset the fence, to not pend infinitely
    vkResetFences(device.logical(), 1, &sync.inFlightFence(currentFrame));

    if (vkQueueSubmit(device.graphicsQueue(), 1, &submitInfo, sync.inFlightFence(currentFrame)) != VK_SUCCESS)
        throw std::runtime_error("failed to submit draw command buffer!");

    // Now, onto the frame presentation !
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    // Swapchains to use
    VkSwapchainKHR swapChains[] = {swapChain.handle()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    // Simpli not used
    presentInfo.pResults = nullptr;

    // Finally, try to present queue
    vkQueuePresentKHR(device.presentQueue(), &presentInfo);

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Application::recreateSwapChain(bool &resized)
{
}

void Application::run()
{
    std::cout << "Application started !\n";
    mainLoop();
    std::cout << "Application closing...\n";
}

void Application::mainLoop()
{
    window.setDrawFrameFunc([this](bool &framebufferResized)
                            { drawFrame(framebufferResized); });

    window.mainLoop(device);
}