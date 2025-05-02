#include "global.hpp"
#include <Application.hpp>

#include <vector>
#include <cstring>

int main()
{
    glfwInit();

    // Remove OpenGL API
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    Application app(true);

    try
    {
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    glfwTerminate();

    return EXIT_SUCCESS;
}