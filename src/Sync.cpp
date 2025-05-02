#include <Sync.hpp>
#include <Device.hpp>

Sync::Sync(const Device &device, uint32_t numImages, uint32_t maxFramesInFlight) : _device(device),
                                                                                   _numImages(numImages),
                                                                                   _maxFramesInFlight(maxFramesInFlight),
                                                                                   _imageAvailable(maxFramesInFlight),
                                                                                   _renderFinished(maxFramesInFlight),
                                                                                   _inFlightFences(maxFramesInFlight),
                                                                                   _imagesInFlight(numImages)
{
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    // Initialize fence to already signaled so it doesn't hang on first frame
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < _maxFramesInFlight; ++i)
    {
        if (vkCreateSemaphore(_device.logical(), &semaphoreInfo, nullptr, &_imageAvailable[i]) != VK_SUCCESS ||
            vkCreateSemaphore(_device.logical(), &semaphoreInfo, nullptr, &_renderFinished[i]) != VK_SUCCESS ||
            vkCreateFence(_device.logical(), &fenceInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create synchronization objects for a frame!");
    }
}

Sync::~Sync()
{
    for (size_t i = 0; i < _maxFramesInFlight; ++i)
    {
        vkDestroySemaphore(_device.logical(), _renderFinished[i], nullptr);
        vkDestroySemaphore(_device.logical(), _imageAvailable[i], nullptr);
        vkDestroyFence(_device.logical(), _inFlightFences[i], nullptr);
    }
}