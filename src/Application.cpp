#include <Application.hpp>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const int MAX_FRAMES_IN_FLIGHT = 2;

Application::Application(bool enableValidationLayers) : window("Test", {WIDTH, HEIGHT}, "Vulkan", enableValidationLayers),
                                                        debugMessenger(window),
                                                        device(window),
                                                        swapChain(device, window),
                                                        defaultRenderPass(device, swapChain),
                                                        graphicsPipeline(device, swapChain, defaultRenderPass, {ShaderInfo("base", true), ShaderInfo("base", false)})
{
}

void Application::drawFrame(bool &resized)
{
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

    window.mainLoop();
    //   vkDeviceWaitIdle(device.logical());
}