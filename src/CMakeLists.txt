target_sources(VkBullshit PRIVATE
    Application.cpp
    Device.cpp
    Messenger.cpp
    Window.cpp
    QueueFamily.cpp
    SwapChain.cpp
    GraphicsPipeline.cpp
    RenderPass.cpp
    CommandPool.cpp
    Sync.cpp
)

add_subdirectory(default)
add_subdirectory(ui)