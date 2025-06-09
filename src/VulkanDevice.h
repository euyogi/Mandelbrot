#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include <string>
#include <set>

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanDevice {
public:
    VulkanDevice(VkInstance instance, VkSurfaceKHR surface);
    ~VulkanDevice();
    
    VulkanDevice(const VulkanDevice&) = delete;
    VulkanDevice& operator=(const VulkanDevice&) = delete;

    VkCommandPool getCommandPool() { return commandPool; }
    VkDevice device() { return device_; }
    VkPhysicalDevice physicalDevice() { return physicalDevice_; }
    VkQueue graphicsQueue() { return graphicsQueue_; }
    VkQueue presentQueue() { return presentQueue_; }
    VkSurfaceKHR surface() { return surface_; }

    SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice_); }
    QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice_); }

private:
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createCommandPool();

    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    VkInstance instance_;
    VkSurfaceKHR surface_;
    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
    VkDevice device_;
    VkCommandPool commandPool;
    
    VkQueue graphicsQueue_;
    VkQueue presentQueue_;

    const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
};
