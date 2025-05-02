#pragma once
#include "global.hpp"
#include "QueueFamily.hpp"

#include <vector>

// Forward declaration
class Window;

class Device
{
private:
    const Window &_window;

    VkPhysicalDevice _physical;
    VkDevice _logical;

    VkQueue _presentQueue;
    VkQueue _graphicsQueue;
    QueueFamily _indices;

    void pickPhysicalDevice();
    bool checkDeviceExtensionSupport(const VkPhysicalDevice &device);
    bool isDeviceSuitable(const VkPhysicalDevice &device);

public:
    Device(const Window &window);

    // Getters
    inline const VkPhysicalDevice &physical() const { return _physical; }
    inline const VkDevice &logical() const { return _logical; }
    inline const QueueFamily &queueFamilyIndices() const { return _indices; }
    inline const VkQueue &graphicsQueue() const { return _graphicsQueue; }
    inline const VkQueue &presentQueue() const { return _presentQueue; }
};