#include <Window.hpp>
#include <Messenger.hpp>
#include "Application.hpp"

const std::vector<const char *> Window::ValidationLayers = {
    "VK_LAYER_KHRONOS_validation"};
const std::vector<const char *> Window::DeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

Window::Window(const std::string appName, const glm::ivec2 size, const char *engineName, const bool enableLayers) :
            _title(appName),
            _size(size),
            _surface(VK_NULL_HANDLE),
            _enableValidationLayers(enableLayers)
{

    // ------------------------- CREATE VULKAN INSTANCE ----------------------
    if (_enableValidationLayers && !CheckValidationLayerSupport())
    {
        throw std::runtime_error("Validation layers requested, but not available!");
    }

    // Determine application info
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = appName.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = engineName;
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // Additional info for instance
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // Manage glfw extensions
    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    // Debug create info
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (_enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
        createInfo.ppEnabledLayerNames = ValidationLayers.data();

        Messenger::PopulateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    // Create instance (or tries to at least lol)
    if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create instance!");
    }

    std::cout << "Successfully created Vulkan instance" << '\n';

    // ------------------------------- CREATE GLFW WINDOWS
    _window = glfwCreateWindow(size.x, size.y, "Vulkan", nullptr, nullptr);

    glfwSetWindowUserPointer(_window, this);
    glfwSetFramebufferSizeCallback(_window, FramebufferResizeCallback);

    if (glfwCreateWindowSurface(_instance, _window, nullptr, &_surface) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create window surface!");
    }
}

Window::~Window()
{
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
    vkDestroyInstance(_instance, nullptr);
    glfwDestroyWindow(_window);
}

void Window::mainLoop()
{
    while (!glfwWindowShouldClose(_window))
    {
        glfwPollEvents();
        _drawFrameFunc(_resized);
    }
}

std::vector<const char *> Window::getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (_enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

void Window::FramebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    Window *win = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    win->_resized = true;
}

bool Window::CheckValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    std::cout << '\n'
              << "Available layers : " << layerCount << ", layers to check : " << ValidationLayers.size() << '\n';

    for (const char *layerName : Window::ValidationLayers)
    {
        bool layerFound = false;

        std::cout << "\tChecking layer " << layerName << "...  ";

        for (const auto &layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
            return false;
        std::cout << "OK" << '\n';
    }

    std::cout << "All layers passed !" << '\n'
              << '\n';

    return true;
}