#pragma once
#include "global.hpp"

#include <Device.hpp>
#include <Window.hpp>
#include <Messenger.hpp>
#include <SwapChain.hpp>
#include <GraphicsPipeline.hpp>
#include <Sync.hpp>

#include <default/DefaultRenderPass.hpp>
#include <default/DefaultCommandPool.hpp>

#include <ui/UI.hpp>

class Application
{
private:
    Window window;
    Messenger debugMessenger;
    Device device;
    SwapChain swapChain;
    DefaultRenderPass defaultRenderPass;
    GraphicsPipeline graphicsPipeline;
    DefaultCommandPool commandPool;
    Sync sync;

    UI interface;

    size_t currentFrame = 0;

    void mainLoop();

    void drawFrame(bool &resized);

    void recreateSwapChain(bool &resized);
    
public:
    Application(bool enableValidationLayers);

    void run();
};