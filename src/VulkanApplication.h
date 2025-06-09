#pragma once

#include "Window.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanPipeline.h"
#include <memory>
#include <vector>

const int MAX_FRAMES_IN_FLIGHT = 2;

class VulkanApplication {
public:
    ~VulkanApplication();
    void run();

    PushConstantData pushConstantData;
    bool isDragging = false;
    glm::vec2 lastMousePos;
    int width = 800;
    int height = 600;

    void recreateSwapChain();

private:
    void initVulkan();
    void mainLoop();

    void createInstance();
    void setupDebugMessenger();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) const;
    std::vector<const char*> getRequiredExtensions() const;
    bool checkValidationLayerSupport() const;
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT,
        VkDebugUtilsMessageTypeFlagsEXT,
        const VkDebugUtilsMessengerCallbackDataEXT*,
        void*
    );

    void createCommandBuffers();
    void createSyncObjects();
    void recordCommandBuffer(uint32_t imageIndex);
    void drawFrame();
    
    std::unique_ptr<Window> window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;
    std::unique_ptr<VulkanDevice> device;
    std::unique_ptr<VulkanSwapchain> swapChain;
    std::unique_ptr<VulkanPipeline> pipeline;
    
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;

    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
};
