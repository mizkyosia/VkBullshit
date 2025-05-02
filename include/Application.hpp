#pragma once
#include "global.hpp"

#include <Device.hpp>
#include <Window.hpp>
#include <Messenger.hpp>
#include <SwapChain.hpp>

class Application
{
private:
    Window window;
    Messenger debugMessenger;
    Device device;
    SwapChain swapChain;

    size_t currentFrame = 0;

    void mainLoop();

    void drawFrame(bool &resized);

    void recreateSwapChain(bool &resized);

public:
    Application(bool enableValidationLayers);

    void run();
};