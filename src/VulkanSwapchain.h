#pragma once

#include "VulkanDevice.h"
#include <vulkan/vulkan.h>
#include <vector>

class VulkanSwapchain {
public:
    VulkanSwapchain(VulkanDevice& device, VkExtent2D windowExtent);
    ~VulkanSwapchain();

    VulkanSwapchain(const VulkanSwapchain&) = delete;
    VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;
    
    VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
    VkRenderPass getRenderPass() { return renderPass; }
    VkImageView getImageView(int index) { return swapChainImageViews[index]; }
    size_t imageCount() { return swapChainImages.size(); }
    VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
    VkExtent2D getSwapChainExtent() { return swapChainExtent; }
    
    VkResult acquireNextImage(uint32_t *imageIndex, VkSemaphore imageAvailableSemaphore);
    VkResult submitCommandBuffers(
        const VkCommandBuffer *buffers,
        const uint32_t *imageIndex,
        VkSemaphore imageAvailableSemaphore,
        VkFence inFlightFence
    );

private:
    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createFramebuffers();
    void createSyncObjects(); 

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkRenderPass renderPass;

    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    VkSwapchainKHR swapChain;

    VulkanDevice& device;
    VkExtent2D windowExtent;
};
