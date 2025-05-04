#include <Application.hpp>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const int MAX_FRAMES_IN_FLIGHT = 5;

Application::Application(bool enableValidationLayers) : window("Test", {WIDTH, HEIGHT}, "Vulkan", enableValidationLayers),
                                                        debugMessenger(window),
                                                        device(window),
                                                        swapChain(device, window),
                                                        defaultRenderPass(device, swapChain),
                                                        graphicsPipeline(device, swapChain, defaultRenderPass, {ShaderInfo("base", true), ShaderInfo("base", false)}),
                                                        commandPool(device, defaultRenderPass, swapChain, graphicsPipeline, 0),
                                                        sync(device, swapChain.numImages(), MAX_FRAMES_IN_FLIGHT),
                                                        interface(window, device, swapChain, graphicsPipeline)
{
}

void Application::drawFrame(bool &resized)
{
    // Wait for the previous frame to be rendered
    vkWaitForFences(device.logical(), 1, &sync.inFlightFence(currentFrame), VK_TRUE, UINT64_MAX);

    // Acquire image for current frame in swapchain. Disables the timeout by putting a very high value
    uint32_t imageIndex;
    auto result = vkAcquireNextImageKHR(device.logical(), swapChain.handle(), UINT64_MAX, sync.imageAvailable(currentFrame), VK_NULL_HANDLE, &imageIndex);

    // Create new swap chain if needed
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChain(resized);
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("Failed to acquire swapchain image");

    // Wait for image in flight
    if (sync.imageInFlight(imageIndex) != VK_NULL_HANDLE)
        vkWaitForFences(device.logical(), 1, &sync.imageInFlight(imageIndex),
                        VK_TRUE, UINT64_MAX);

    // Update semaphores
    sync.imageInFlight(imageIndex) = sync.inFlightFence(currentFrame);

    // Reset the current command buffers, so that they may be used again
    vkResetCommandBuffer(commandPool.command(currentFrame), 0);
    vkResetCommandBuffer(interface.command(currentFrame), 0);

    // Re-record the command buffers for the current frame/image
    interface.recordCommandBuffers(currentFrame);
    commandPool.recordCommandBuffer(currentFrame);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {sync.imageAvailable(currentFrame)};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    // Set command buffers to give (UI + simulation)
    VkCommandBuffer buffers[] = {commandPool.command(currentFrame), interface.command(currentFrame)};
    submitInfo.commandBufferCount = 2;
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
    // Simply not used
    presentInfo.pResults = nullptr;

    // Finally, try to present queue
    result = vkQueuePresentKHR(device.presentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || resized)
    {
        recreateSwapChain(resized);
        resized = false;
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present swap chain image");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
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
                            {
                                interface.draw();
                                drawFrame(framebufferResized); });

    window.mainLoop(device);
}

void Application::recreateSwapChain(bool &resized)
{
    resized = true;

    glm::ivec2 size;
    window.framebufferSize(size);
    while (size[0] == 0 || size[1] == 0)
    {
        window.framebufferSize(size);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device.logical());

    swapChain.recreate();
    defaultRenderPass.recreate();
    graphicsPipeline.recreate();
    commandPool.recreateCommandBuffers();

    // interface.recreate();

    defaultRenderPass.cleanupOld();
    swapChain.cleanupOld();
}