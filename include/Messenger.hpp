#pragma once
#include <global.hpp>

// Forward declaration
class Window;

class Messenger
{
private:
    const Window &_window;
    VkDebugUtilsMessengerEXT _debugMessenger;

public:
    Messenger(Window &widnow);
    ~Messenger() noexcept(false);

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData)
    {
        std::cerr << "\tLayer message : " << pCallbackData->pMessage << '\n';
        return VK_FALSE;
    }

    inline const VkDebugUtilsMessengerEXT &handle() const { return _debugMessenger; }

    static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
};