#include <Device.hpp>
#include <Window.hpp>
#include <QueueFamily.hpp>
#include <SwapChain.hpp>

#include <set>

void Device::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(_window.instance(), &deviceCount, nullptr);

    // No vlk support :(
    if (deviceCount == 0)
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(_window.instance(), &deviceCount, devices.data());

    for (const auto &device : devices)
    {
        if (isDeviceSuitable(device))
        {
            _physical = device;
            break;
        }
    }

    if (_physical == VK_NULL_HANDLE)
    {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

Device::Device(const Window &window) : _window(window), _physical(VK_NULL_HANDLE), _logical(VK_NULL_HANDLE), _presentQueue(VK_NULL_HANDLE), _graphicsQueue(VK_NULL_HANDLE)
{
    pickPhysicalDevice();

    _indices = QueueFamily(_physical, _window.surface());

    // Setup queue families for device
    std::set<uint32_t> uniqueQueueFamilies = {_indices.graphicsFamily.value(),
                                              _indices.presentFamily.value()};
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    float priority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        createInfo.queueFamilyIndex = queueFamily;
        createInfo.queueCount = 1;
        createInfo.pQueuePriorities = &priority;
        queueCreateInfos.push_back(createInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};

    // Setup logical device
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount =
        static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    // Manage GLFW extensions
    createInfo.enabledExtensionCount = static_cast<uint32_t>(Window::DeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = Window::DeviceExtensions.data();

    if (_window.enabledValidationLayers())
    {
        createInfo.enabledLayerCount =
            static_cast<uint32_t>(Window::ValidationLayers.size());
        createInfo.ppEnabledLayerNames = Window::ValidationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(_physical, &createInfo, nullptr, &_logical) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device!");
    }

    // Get handles for graphics and presentation queues
    vkGetDeviceQueue(_logical, _indices.graphicsFamily.value(), 0,
                     &_graphicsQueue);
    vkGetDeviceQueue(_logical, _indices.presentFamily.value(), 0,
                     &_presentQueue);
}

bool Device::isDeviceSuitable(const VkPhysicalDevice &device)
{
    QueueFamily indices(device, _window.surface());

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = SwapChain::QuerySwapChainSupport(device, _window.surface());
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

bool Device::checkDeviceExtensionSupport(const VkPhysicalDevice &device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(Window::DeviceExtensions.begin(), Window::DeviceExtensions.end());

    for (const auto &extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}