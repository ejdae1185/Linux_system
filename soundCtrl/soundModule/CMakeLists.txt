set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# 하위 디렉토리인 recoding을 추가
add_subdirectory(recoding)

# soundModule 라이브러리를 정의
add_library(soundModule SHARED soundModule.c)

# ALSA 라이브러리 링크
find_package(ALSA REQUIRED)
if(ALSA_FOUND)
    include_directories(${ALSA_INCLUDE_DIRS})
    target_link_libraries(soundModule recoding ${ALSA_LIBRARIES})
endif()

# Include directories 설정
target_include_directories(soundModule PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/recoding
)