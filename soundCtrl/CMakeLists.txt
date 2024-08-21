cmake_minimum_required(VERSION 3.0)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/messageModule
)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)



set(modules
    messageModule
    soundModule
    drawModule
)

foreach(module ${modules})
    add_subdirectory(${module})
endforeach()


add_executable(main main.c)

target_link_libraries(
    main 
    messageModule 
    soundModule 
    drawModule
)

target_include_directories(main PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/messageModule
    ${CMAKE_CURRENT_SOURCE_DIR}/soundModule
    ${CMAKE_CURRENT_SOURCE_DIR}/soundModule/recoding
    ${CMAKE_CURRENT_SOURCE_DIR}/drawModule
)
