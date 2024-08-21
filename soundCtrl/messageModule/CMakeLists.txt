# make Library  
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(messageModule SHARED messageModule.c)

target_include_directories(messageModule PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
