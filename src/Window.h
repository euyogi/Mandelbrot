#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <glm/glm.hpp>

class VulkanApplication;

class Window {
public:
    Window(int w, int h, std::string name);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool shouldClose() { return glfwWindowShouldClose(window); }
    
    GLFWwindow* getGLFWwindow() const { return window; }

    void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
    void setupCallbacks(VulkanApplication* app);

private:
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);

    int width;
    int height;
    std::string windowName;
    GLFWwindow* window;
};
