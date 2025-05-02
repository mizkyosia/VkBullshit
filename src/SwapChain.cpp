#include <SwapChain.hpp>
#include <Window.hpp>
#include <Device.hpp>
#include <QueueFamily.hpp>

#include <cmath>
#include <algorithm>

void SwapChain::createSwapChain()
{
    // Fetch support details
    _supportDetails = QuerySwapChainSupport(_device.physical(), _window.surface());
    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(_supportDetails.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(_supportDetails.presentModes);
    VkExtent2D extent = ChooseSwapExtent(_supportDetails.capabilities, _window);

    // 1 more image than the minimum is usually enough to not have too much wait time, while keeping a light enough load
    uint32_t imageCount = _supportDetails.capabilities.minImageCount + 1;

    // If the max image count is 0, there is no maximum
    if (_supportDetails.capabilities.maxImageCount > 0 && imageCount > _supportDetails.capabilities.maxImageCount)
    {
        imageCount = _supportDetails.capabilities.maxImageCount;
    }

    // And here we go with another build struct
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = _window.surface();
    createInfo.minImageCount = imageCount;

    // We re-use our fetched & calculated data
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamily indices(_device.physical(), _window.surface());
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    // If the 2 families differ, use concurrent mode to avoid ownership shenanigans
    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    // No special transform on images
    createInfo.preTransform = _supportDetails.capabilities.currentTransform;

    // Dictates alpha blend mode
    // IMPORTANT IF MAKING SEMI-TRANSPARENT WINDOWS
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    // Sets present mode and enables clipping (not rendering pixels not shown on screen)
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    // Set the old swap chain
    createInfo.oldSwapchain = _oldSwapChain;

    if (vkCreateSwapchainKHR(_device.logical(), &createInfo, nullptr, &_swapChain) != VK_SUCCESS)
        throw std::runtime_error("Failed to create swap chain!");

    std::cout << "Successfully created swapchain" << std::endl;
}

VkSurfaceFormatKHR SwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    // Try to find SRGB available format
    for (const auto &availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }
    // Otherwise, defaults to the first format
    return availableFormats[0];
}

VkPresentModeKHR SwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
    // Choose Mailbox if available (triple buffering is kinda nice)
    for (const auto &availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    // FIFO is always available as a fallback
    return VK_PRESENT_MODE_FIFO_KHR;
}

SwapChain::SwapChain(const Device &device, const Window &window) : _device(device), _window(window), _oldSwapChain(VK_NULL_HANDLE)
{
    createSwapChain();
}

SwapChain::~SwapChain()
{
    // Destroy the swapchain
    vkDestroySwapchainKHR(_device.logical(), _swapChain, nullptr);
}

SwapChainSupportDetails SwapChain::QuerySwapChainSupport(const VkPhysicalDevice &device, const VkSurfaceKHR &surface)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkExtent2D SwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, const Window &window)
{
    // If window resolution is used normally
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        // Fetch current height & width
        VkExtent2D actualExtent = {
            static_cast<uint32_t>(window.size().x),
            static_cast<uint32_t>(window.size().y)};

        // Get closest available value
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}