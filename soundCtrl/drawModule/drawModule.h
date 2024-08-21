
#ifndef DRAWMODULE_H
#define DRAWMODULE_H
#include <type.h>

#include <GLFW/glfw3.h>

// 버튼의 크기와 위치 정의
typedef struct
{
    float x, y, width, height;
} Button;

uint32_t *drawModule();

#endif // DRAWMODULE_H
