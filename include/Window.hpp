#pragma once
#include "global.hpp"

#include <glm/glm.hpp>
#include <functional>
#include <cstring>

// Forward declaration
class Device;

class Window
{
private:
/// @brief Window title
    std::string _title;
    /// @brief Dimensions of the window
    glm::ivec2 _size;
    /// @brief Our pretty window :3
    GLFWwindow *_window;
    /// @brief Actual surface to draw onto
    VkSurfaceKHR _surface;
    
    /// @brief Has the window been resized this frame ?
    bool _resized;
    
    /// @brief Callback for the main loop, draws each frame
    std::function<void(bool &)> _drawFrameFunc;
    /// @brief Actual Vulkan instance used for the surface
    VkInstance _instance;
    /// @brief Are the validation layers enabled ?
    bool _enableValidationLayers;

public:
    /// @brief Global validation layers
    static const std::vector<const char *> ValidationLayers;
    /// @brief Global device extensions
    static const std::vector<const char *> DeviceExtensions;

    Window(const std::string appName, const glm::ivec2 size, const char *engineName, const bool enableLayers);
    ~Window();

    /// @brief Main loop of the window
    void mainLoop(const Device& device);

    /// @brief Fetch required extensions for Vulkan
    std::vector<const char *> getRequiredExtensions();

    /// @brief Changes the "draw frame" function, which is called each frame by the application
    /// @param func
    inline void setDrawFrameFunc(const std::function<void(bool &)> &func)
    {
        _drawFrameFunc = func;
    }

    inline void framebufferSize(glm::ivec2 &size) const
    {
        glfwGetFramebufferSize(_window, &size[0], &size[1]);
    }

    // Getters
    inline const glm::ivec2 &size() const { return _size; }
    inline const GLFWwindow *window() const { return _window; }
    inline const VkInstance &instance() const { return _instance; }
    inline const VkSurfaceKHR &surface() const { return _surface; }
    inline bool enabledValidationLayers() const { return _enableValidationLayers; }

    /// @brief Handle frame buffer resize (window resize)
    /// @param window
    /// @param width
    /// @param height
    static void FramebufferResizeCallback(GLFWwindow *window, int width, int height);

    /// @brief Checks if validation layers are all supported
    /// @return
    static bool CheckValidationLayerSupport();
};
