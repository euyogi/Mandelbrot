#include "VulkanApplication.h"
#include <iostream>
#include <exception>
#include <cstdlib>

int main() {
    VulkanApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}