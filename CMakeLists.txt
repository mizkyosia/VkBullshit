cmake_minimum_required(VERSION 3.15.0)
project(VkBullshit VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)

add_executable(VkBullshit main.cpp)
add_subdirectory(src)

# Set compiler options
target_compile_options(VkBullshit PRIVATE -Wall)

# Find libraries
find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

# Link libraries
target_link_libraries(VkBullshit Vulkan::Vulkan)
target_link_libraries(VkBullshit glfw)
target_include_directories(VkBullshit PRIVATE ${PROJECT_SOURCE_DIR}/include)