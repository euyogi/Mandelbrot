#pragma once

#include "VulkanDevice.h"
#include <string>
#include <vector>
#include <glm/glm.hpp>

struct PushConstantData {
    glm::vec2 offset;
    float zoom;
    float aspectRatio;
};

class VulkanPipeline {
public:
    VulkanPipeline(VulkanDevice& device, const std::string& vertFilepath, const std::string& fragFilepath, VkRenderPass renderPass);
    ~VulkanPipeline();

    VulkanPipeline(const VulkanPipeline&) = delete;
    VulkanPipeline& operator=(const VulkanPipeline&) = delete;
    
    void bind(VkCommandBuffer commandBuffer);
    VkPipelineLayout getLayout() const { return pipelineLayout_; }

    static std::vector<char> readFile(const std::string& filename);
    
    static void cleanupShaderModules(VkDevice);

    static VkShaderModule vertShaderModule;
    static VkShaderModule fragShaderModule;
    
private:
    void createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath, VkRenderPass renderPass);
    VkShaderModule createShaderModule(const std::vector<char>& code);

    VulkanDevice& device_;
    VkPipeline graphicsPipeline_;
    VkPipelineLayout pipelineLayout_;
};
