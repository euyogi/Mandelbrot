#include "Window.h"
#include "VulkanApplication.h"
#include <stdexcept>

Window::Window(int w, int h, std::string name) : width(w), height(h), windowName(name) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
    if (!window) throw std::runtime_error("failed to create GLFW window!");
}

Window::~Window() {
    glfwDestroyWindow(window);
}

void Window::setupCallbacks(VulkanApplication* app) {
    glfwSetWindowUserPointer(window, app);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);
}

void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
    if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
        throw std::runtime_error("failed to create window surface!");
}

void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(window));
    app->recreateSwapChain();
}

void Window::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    auto app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(window));
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    glm::vec2 currentMousePos = {float(xpos), float(ypos)};
    currentMousePos = 2.0f * currentMousePos / glm::vec2(app->width, app->height) - 1.0f;
    float zoomFactor = powf(1.1f, float(yoffset));
    float currentZoom = app->pushConstantData.zoom;
    glm::vec2 offsetDelta = currentMousePos / currentZoom * (1.0f - (1.0f / zoomFactor));
    app->pushConstantData.offset += offsetDelta;
    app->pushConstantData.zoom *= zoomFactor;
}

void Window::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            app->isDragging = true;
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            app->lastMousePos = {float(xpos), float(ypos)};
        } else if (action == GLFW_RELEASE)
            app->isDragging = false;
    }
}

void Window::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    auto app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(window));
    if (app->isDragging) {
        glm::vec2 currentMousePos = {float(xpos), float(ypos)};
        glm::vec2 delta = currentMousePos - app->lastMousePos;
        delta = 2.0f * delta / glm::vec2(app->width, app->height);
        float currentZoom = app->pushConstantData.zoom;
        app->pushConstantData.offset -= delta / currentZoom;
        app->lastMousePos = currentMousePos;
    }
}
