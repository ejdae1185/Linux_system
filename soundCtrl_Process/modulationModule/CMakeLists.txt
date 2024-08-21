cmake_minimum_required(VERSION 3.0)
project(modulationModule)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

find_package(ALSA REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(SOX REQUIRED sox)

include_directories(${SOX_INCLUDE_DIRS})
add_executable(modulationModule modulationModule.c)

target_link_libraries(modulationModule ${SOX_LIBRARIES})
if(ALSA_FOUND)
    include_directories(${ALSA_INCLUDE_DIRS})
    target_link_libraries(modulationModule ${ALSA_LIBRARIES})
endif()


