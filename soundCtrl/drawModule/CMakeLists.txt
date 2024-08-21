
add_library(drawModule SHARED drawModule.c)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
target_include_directories(drawModule PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(drawModule PUBLIC OpenGL::GL glfw)