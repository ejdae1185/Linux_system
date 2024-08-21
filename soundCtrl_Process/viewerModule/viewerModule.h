
#ifndef DRAWMODULE_H
#define DRAWMODULE_H
#include <type.h>
#include <system.h>
#include <GLFW/glfw3.h>
typedef struct
{
    float x, y, width, height;
} Button;

uint32_t *drawModule();
pid_t getPidByName(const char *processName);
#endif // DRAWMODULE_H
