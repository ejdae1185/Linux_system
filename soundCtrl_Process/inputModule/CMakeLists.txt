cmake_minimum_required(VERSION 3.0)

project(InputModule)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

find_package(ALSA REQUIRED)

add_executable(InputModule inputModule.c)

if(ALSA_FOUND)
    include_directories(${ALSA_INCLUDE_DIRS})
    target_link_libraries(InputModule ${ALSA_LIBRARIES})
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

