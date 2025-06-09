#include "VulkanApplication.h"
#include <stdexcept>
#include <iostream>
#include <vector>
#include <cstring>

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) func(instance, debugMessenger, pAllocator);
}

void VulkanApplication::run() {
    initVulkan();
    mainLoop();
}

void VulkanApplication::initVulkan() {
    glfwInit(); 

    pushConstantData.offset = {-0.25f, 0.0f};
    pushConstantData.zoom = 1.0f;
    pushConstantData.aspectRatio = float(width) / float(height);

    window = std::make_unique<Window>(width, height, "Vulkan Shader Viewer");

    createInstance();
    setupDebugMessenger();
    
    window->setupCallbacks(this);
    window->createWindowSurface(instance, &surface);
    device = std::make_unique<VulkanDevice>(instance, surface);
    recreateSwapChain(); 
    createCommandBuffers();
    createSyncObjects();
}

VulkanApplication::~VulkanApplication() {
    vkDeviceWaitIdle(device->device());

    VulkanPipeline::cleanupShaderModules(device->device());
    
    pipeline.reset();
    swapChain.reset();
    
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device->device(), imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device->device(), inFlightFences[i], nullptr);
    }

    vkFreeCommandBuffers(
        device->device(), 
        device->getCommandPool(), 
        static_cast<uint32_t>(commandBuffers.size()), 
        commandBuffers.data());

    device.reset();

    if (enableValidationLayers)
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    window.reset();

    glfwTerminate(); 
}

void VulkanApplication::recreateSwapChain() {
    int currentWidth = 0, currentHeight = 0;
    glfwGetFramebufferSize(window->getGLFWwindow(), &currentWidth, &currentHeight);
    while (currentWidth == 0 || currentHeight == 0) {
        glfwGetFramebufferSize(window->getGLFWwindow(), &currentWidth, &currentHeight);
        glfwWaitEvents();
    }
    width = currentWidth;
    height = currentHeight;

    vkDeviceWaitIdle(device->device());
        
    pipeline.reset();
    swapChain.reset();
    
    VkExtent2D newExtent = { static_cast<uint32_t>(currentWidth), static_cast<uint32_t>(currentHeight) };
    swapChain = std::make_unique<VulkanSwapchain>(*device, newExtent);
    pipeline = std::make_unique<VulkanPipeline>(*device, "shaders/vert.spv", "shaders/frag.spv", swapChain->getRenderPass());
    
    pushConstantData.aspectRatio = swapChain->getSwapChainExtent().width / (float)swapChain->getSwapChainExtent().height;
}

void VulkanApplication::mainLoop() {
    while (!window->shouldClose()) {
        glfwPollEvents();
        drawFrame();
    }
}

void VulkanApplication::drawFrame() {
    vkWaitForFences(device->device(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    auto result = swapChain->acquireNextImage(&imageIndex, imageAvailableSemaphores[currentFrame]);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        vkResetFences(device->device(), 1, &inFlightFences[currentFrame]);
        return;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("failed to acquire swap chain image!");

    vkResetFences(device->device(), 1, &inFlightFences[currentFrame]);
    vkResetCommandBuffer(commandBuffers[currentFrame], 0);
    recordCommandBuffer(imageIndex);

    result = swapChain->submitCommandBuffers(
        &commandBuffers[currentFrame], 
        &imageIndex, 
        imageAvailableSemaphores[currentFrame], 
        inFlightFences[currentFrame]);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        recreateSwapChain();
    else if (result != VK_SUCCESS)
        throw std::runtime_error("failed to present swap chain image!");
    
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanApplication::recordCommandBuffer(uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("failed to begin recording command buffer!");
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = swapChain->getRenderPass();
    renderPassInfo.framebuffer = swapChain->getFrameBuffer(imageIndex);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChain->getSwapChainExtent();
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    pipeline->bind(commandBuffers[currentFrame]);
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChain->getSwapChainExtent().width);
    viewport.height = static_cast<float>(swapChain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChain->getSwapChainExtent();
    vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

    vkCmdPushConstants(commandBuffers[currentFrame], pipeline->getLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData), &pushConstantData);
    
    vkCmdDraw(commandBuffers[currentFrame], 3, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffers[currentFrame]);
    if (vkEndCommandBuffer(commandBuffers[currentFrame]) != VK_SUCCESS)
        throw std::runtime_error("failed to record command buffer!");
}

void VulkanApplication::createInstance() {
    if (enableValidationLayers && !checkValidationLayerSupport())
        throw std::runtime_error("validation layers requested, but not available!");
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Shader Viewer";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
        throw std::runtime_error("failed to create instance!");
}

void VulkanApplication::setupDebugMessenger() {
    if (!enableValidationLayers) return;
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);
    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
        throw std::runtime_error("failed to set up debug messenger!");
}

void VulkanApplication::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) const {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr; 
}

std::vector<const char*> VulkanApplication::getRequiredExtensions() const {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if (enableValidationLayers)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    return extensions;
}

bool VulkanApplication::checkValidationLayerSupport() const {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    for (const char* layerName : validationLayers) {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers)
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        if (!layerFound) return false;
    }
    return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanApplication::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void*) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

void VulkanApplication::createCommandBuffers() {
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = device->getCommandPool();
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();
    if (vkAllocateCommandBuffers(device->device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate command buffers!");
}

void VulkanApplication::createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        if (vkCreateSemaphore(device->device(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device->device(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create sync objects for a frame!");
        }
}
