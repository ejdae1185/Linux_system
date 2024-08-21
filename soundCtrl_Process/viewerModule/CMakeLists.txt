cmake_minimum_required(VERSION 3.0)

project(ViewerModule)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Freetype REQUIRED)
find_package(GLUT REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/controlModule)
message("assssssssssssssssssssssss"${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(controlModule)
# Define the executable target
add_executable(ViewerModule viewerModule.c )

message(${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(ViewerModule PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(ViewerModule PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_include_directories(ViewerModule PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/controlModule)
target_link_libraries(ViewerModule PUBLIC
 OpenGL::GL 
 glfw 
 controlModule)
 target_link_libraries(ViewerModule PRIVATE ${FREETYPE_LIBRARIES} ${GLUT_LIBRARIES})
