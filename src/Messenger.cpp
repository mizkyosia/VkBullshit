#include <Messenger.hpp>
#include <Window.hpp>

Messenger::Messenger(Window &window) : _window(window)
{
    if (_window.enabledValidationLayers())
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        PopulateDebugMessengerCreateInfo(createInfo);

        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_window.instance(), "vkCreateDebugUtilsMessengerEXT");
        if (func == nullptr)
            throw std::runtime_error("Debug Utils Messenger extension currently unavailable");

        if (func(_window.instance(), &createInfo, nullptr, &_debugMessenger) != VK_SUCCESS)
            throw std::runtime_error("Failed to initialize debug messenger");
    }
}

Messenger::~Messenger() noexcept(false)
{
    if (_window.enabledValidationLayers())
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_window.instance(), "vkDestroyDebugUtilsMessengerEXT");

        if (func == nullptr)
            throw std::runtime_error("Failed to destroy Debug Utils Messenger");

        func(_window.instance(), _debugMessenger, nullptr);
    }
}

void Messenger::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
}
