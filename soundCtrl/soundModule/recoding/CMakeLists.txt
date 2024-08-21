set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# PkgConfig 모듈을 찾습니다.
find_package(PkgConfig REQUIRED)

# pkg-config를 통해 SoX를 찾습니다.
pkg_check_modules(SOX REQUIRED sox)


# recoding 라이브러리를 정의합니다.
add_library(recoding SHARED writeSound.c)
include_directories(${SOX_INCLUDE_DIRS})
target_link_libraries(recoding ${SOX_LIBRARIES})

# ALSA 라이브러리 포함
find_package(ALSA REQUIRED)
if(ALSA_FOUND)
    include_directories(${ALSA_INCLUDE_DIRS})
    target_link_libraries(recoding ${ALSA_LIBRARIES})
endif()

# Include directories 설정
target_include_directories(recoding PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)
