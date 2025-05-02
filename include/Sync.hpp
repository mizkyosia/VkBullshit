#include "global.hpp"
#include <vector>

// Forward declaration
class Device;

class Sync
{
private:
    const Device &_device;

    uint32_t _numImages, _maxFramesInFlight;

    std::vector<VkSemaphore> _imageAvailable;
    std::vector<VkSemaphore> _renderFinished;
    std::vector<VkFence> _inFlightFences;
    std::vector<VkFence> _imagesInFlight;

public:
    Sync(const Device &device, uint32_t numImages,
                uint32_t maxFramesInFlight);
    ~Sync();

    // Getters
    inline VkSemaphore &imageAvailable(uint32_t index) { return _imageAvailable[index]; }
    inline VkSemaphore &renderFinished(uint32_t index) { return _renderFinished[index]; }
    inline VkFence &inFlightFence(uint32_t index) { return _inFlightFences[index]; }
    inline VkFence &imageInFlight(uint32_t index) { return _imagesInFlight[index]; }
};