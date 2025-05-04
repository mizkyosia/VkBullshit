#include <ui/UI.hpp>
#include <ui/UIRenderPass.hpp>

void UI::createImGuiDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> pool_sizes = {
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    uint32_t size = static_cast<uint32_t>(pool_sizes.size());
    pool_info.maxSets = 1000 * size;
    pool_info.poolSizeCount = size;
    pool_info.pPoolSizes = pool_sizes.data();

    if (vkCreateDescriptorPool(_device.logical(), &pool_info, nullptr,
                               &_imGuiDescriptorPool) != VK_SUCCESS)
        throw std::runtime_error("Cannot allocate UI descriptor pool!");
}

void UI::recreate()
{
    _renderPass.recreate();
    _commandPool.recreateCommandBuffers();
    _renderPass.cleanupOld();
};

UI::UI(const Window &window, const Device &device, const SwapChain &swapChain, const GraphicsPipeline &graphicsPipeline) : _window(window),
                                                                                                                           _device(device),
                                                                                                                           _swapChain(swapChain),
                                                                                                                           _graphicsPipeline(graphicsPipeline),
                                                                                                                           _renderPass(_device, _swapChain),
                                                                                                                           _commandPool(_device, _renderPass, _swapChain, _graphicsPipeline, 0)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Initialize descriptor pool for ImGui-specific data
    createImGuiDescriptorPool();

    QueueFamily indices = QueueFamily(_device.physical(), _window.surface());

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForVulkan((GLFWwindow *)_window.window(), true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = _window.instance();
    init_info.PhysicalDevice = _device.physical();
    init_info.Device = _device.logical();
    init_info.QueueFamily = indices.graphicsFamily.value();
    init_info.Queue = _device.graphicsQueue();
    init_info.DescriptorPool = _imGuiDescriptorPool;
    init_info.MinImageCount = _swapChain.numImages();
    init_info.ImageCount = _swapChain.numImages();
    init_info.RenderPass = _renderPass.handle();
    ImGui_ImplVulkan_Init(&init_info);
}

UI::~UI()
{
    // Resources to destroy when the program ends
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(_device.logical(), _imGuiDescriptorPool, nullptr);
}

void UI::draw()
{
    // Start the Dear ImGui frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static float f = 0.0f;
    static int counter = 0;

    ImGui::Begin("Renderer Options");
    ImGui::Text("This is some useful text.");
    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
    if (ImGui::Button("Button"))
    {
        counter++;
    }
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();

    ImGui::Render();
}