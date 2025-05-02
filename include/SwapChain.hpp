#pragma once

#include <vector>

#include "global.hpp"

class Window;
class Device;

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class SwapChain
{
private:
    const Device &_device;
    const Window &_window;

    SwapChainSupportDetails _supportDetails;
    VkSwapchainKHR _swapChain;
    VkSwapchainKHR _oldSwapChain;

    // Swap chain image handles
    std::vector<VkImage> _images;
    std::vector<VkImageView> _imageViews;

    VkFormat _imageFormat;
    VkExtent2D _extent;

    void createSwapChain();
    void createImageViews();

    void destroyImageViews();

    static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR> &availableFormats);
    static VkPresentModeKHR ChooseSwapPresentMode(
        const std::vector<VkPresentModeKHR> &availablePresentModes);


public:
    explicit SwapChain(const Device &device, const Window &window);
    ~SwapChain();

    void recreate();
    void cleanupOld();


    // Getters
    inline const VkSwapchainKHR &handle() const { return _swapChain; }
    inline const VkFormat &imageFormat() const { return _imageFormat; }
    inline const VkExtent2D &extent() const { return _extent; }
    inline size_t numImages() const { return _images.size(); }
    inline size_t numImageViews() const { return _imageViews.size(); }

    inline const SwapChainSupportDetails &supportDetails() const
    {
        return _supportDetails;
    }
    inline VkImageView imageView(uint32_t index) const
    {
        return _imageViews[index];
    }

    static SwapChainSupportDetails
    QuerySwapChainSupport(const VkPhysicalDevice &device,
                          const VkSurfaceKHR &surface);
    static VkExtent2D
    ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities,
                     const Window &window);


};