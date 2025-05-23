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
    _extent = ChooseSwapExtent(_supportDetails.capabilities, _window);
    _imageFormat = surfaceFormat.format;

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
    createInfo.imageExtent = _extent;
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

    // Get swap chain images
    vkGetSwapchainImagesKHR(_device.logical(), _swapChain, &imageCount, nullptr);
    _images.resize(imageCount);
    vkGetSwapchainImagesKHR(_device.logical(), _swapChain, &imageCount, _images.data());
}

void SwapChain::createImageViews()
{
    // Resize to the number of images only
    _imageViews.resize(_images.size());

    // Now, create the actual images
    for (size_t i = 0; i < _images.size(); i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = _images[i];

        // Type of the image :
        // - 1D texture
        // - 2D texture
        // - 3D texture
        // - Cubemap
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = _imageFormat;

        // Allows for tampering with color channels (monochrome effects etc.)
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        // No mipmapping or layers
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(_device.logical(), &createInfo, nullptr, &_imageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image views!");
        }
    }
}

void SwapChain::destroyImageViews()
{
    for (auto imageView : _imageViews)
    {
        vkDestroyImageView(_device.logical(), imageView, nullptr);
    }
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
    createImageViews();
}

SwapChain::~SwapChain()
{
    if (_swapChain != VK_NULL_HANDLE)
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

void SwapChain::recreate()
{
    destroyImageViews();
    cleanupOld();
    _oldSwapChain = _swapChain;
    createSwapChain();
    createImageViews();
}

void SwapChain::cleanupOld()
{ // Destroy old swap chain if it exists
    if (_oldSwapChain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(_device.logical(), _oldSwapChain, nullptr);
        _oldSwapChain = VK_NULL_HANDLE;
    }
}
