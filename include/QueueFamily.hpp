#pragma once

#include "global.hpp"
#include <optional>
#include <vector>

struct QueueFamily
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    QueueFamily();
    QueueFamily(VkPhysicalDevice device, VkSurfaceKHR surface);

    bool isComplete();
};