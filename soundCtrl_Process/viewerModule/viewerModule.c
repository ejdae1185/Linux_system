#include "viewerModule.h"
#include <controlModule.h>
#include <GL/glut.h>
// #include <ft2build.h>
// #include FT_FREETYPE_H
// FT_Library ft;
// FT_Face face;
pid_t inputModule;
pid_t outputModule;
pid_t modulationModule;

pid_t inputModulePid = 0;
pid_t modulationModulePid = 0;
bool isVolumeInputActive = false; // 볼륨 입력 활성화 여부

Button recordButton = {100.0f, 100.0f, 100.0f, 50.0f};
Button stopButton = {300.0f, 100.0f, 100.0f, 50.0f};
Button openButton = {500.0f, 100.0f, 100.0f, 50.0f};
Button modulationButton = {500.0f, 200.0f, 100.0f, 50.0f};
Button volumeInputBox = {50.0f, 500.0f, 200.0f, 30.0f};
Button reset = {
    600.0f,
    500.0f,
    60.0f,
    30.0f}; // 볼륨 입력 박스 추가
// 볼륨 입력 박스 추가

int volumeLevel = 0; // 소리 높이 조절 변수

bool isRecording = false;
volatile int shouldTerminate = 0;

void text_input_callback(GLFWwindow *window, unsigned int codepoint)
{
    static char inputText[256] = {0};
    static int inputLength = 0;

    if (isVolumeInputActive)
    {
        // 숫자 입력 처리
        if (codepoint >= '0' && codepoint <= '9' && inputLength < sizeof(inputText) - 1)
        {
            inputText[inputLength++] = (char)codepoint;
            inputText[inputLength] = '\0';
            volumeLevel = atoi(inputText);
            printf("Volume level: %d\n", volumeLevel);
        }
        else if (codepoint == 8 && inputLength > 0) // 백스페이스 처리
        {
            inputText[--inputLength] = '\0';
            volumeLevel = atoi(inputText);
            printf("Volume level: %d\n", volumeLevel);
        }
    }
}

pid_t getPidByName(const char *processName)
{
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "pgrep -o %s", processName); // -o 옵션은 가장 오래된 프로세스의 PID를 반환

    FILE *fp = popen(cmd, "r");
    if (fp == NULL)
    {
        perror("popen failed");
        return -1;
    }

    char pid_str[16];
    if (fgets(pid_str, sizeof(pid_str), fp) != NULL)
    {
        pid_t pid = (pid_t)strtoul(pid_str, NULL, 10);
        pclose(fp);
        return pid;
    }

    pclose(fp);
    return -1; // 프로세스를 찾을 수 없으면 -1 반환
}

int checkButtonClick(Button button, float mouseX, float mouseY)
{
    return (mouseX > button.x && mouseX < button.x + button.width &&
            mouseY > button.y && mouseY < button.y + button.height);
}

// void drawText(const char *text, float x, float y, float scale)
// {
//     glPushMatrix();
//     glTranslatef(x, y, 0);
//     glScalef(scale, scale, 1);

//     for (const char *p = text; *p; p++)
//     {
//         if (FT_Load_Char(face, *p, FT_LOAD_RENDER))
//         {
//             continue;
//         }

//         glBegin(GL_QUADS);
//         glTexCoord2f(0, 0);
//         glVertex2f(0, 0);
//         glTexCoord2f(1, 0);
//         glVertex2f(face->glyph->bitmap.width, 0);
//         glTexCoord2f(1, 1);
//         glVertex2f(face->glyph->bitmap.width, face->glyph->bitmap.rows);
//         glTexCoord2f(0, 1);
//         glVertex2f(0, face->glyph->bitmap.rows);
//         glEnd();

//         glTranslatef(face->glyph->advance.x >> 6, 0, 0);
//     }

//     glPopMatrix();
// }
void drawTextBox(float x, float y, float width, float height, const char *text)
{
    // 텍스트 박스 배경 그리기
    glColor3f(1.0f, 1.0f, 1.0f); // 흰색 배경
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // 텍스트 그리기
    glColor3f(0.0f, 0.0f, 0.0f);           // 검은색 텍스트
    glRasterPos2f(x + 5, y + height - 15); // 텍스트 위치 조정
    for (const char *c = text; *c != '\0'; c++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

void drawButton(Button button, const char *label)
{
    // 버튼 사각형 그리기
    glBegin(GL_QUADS);
    glVertex2f(button.x, button.y);
    glVertex2f(button.x + button.width, button.y);
    glVertex2f(button.x + button.width, button.y + button.height);
    glVertex2f(button.x, button.y + button.height);
    glEnd();

    // 버튼 텍스트 그리기 (여기서는 생략, OpenGL 텍스트 렌더링을 추가해야 함)
    // drawText(label, button.x + button.width / 2 - 20, button.y + button.height / 2 - 10, 10.0f);
}

void window_close_callback(GLFWwindow *window)
{
    shouldTerminate = 1;
    glfwSetWindowShouldClose(window, GLFW_TRUE);
    inputModule = getPidByName("inputModule");
    outputModule = getPidByName("outputModule");
    modulationModule = getPidByName("modulationModule");

    kill(inputModulePid, SIGKILL);
    // kill(outputModule, SIGKILL);
    // kill(modulationModule, SIGKILL);

    printf("Viewer module is terminated\n");
    printf("send kill -9 inputModulePid = %d\n", inputModulePid);
    exit(0);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        // 버튼 클릭 체크
        if (checkButtonClick(recordButton, (float)mouseX, (float)mouseY))
        {
            // 녹음 시작 코드
            printf("Record button clicked\n");
            recode(inputModulePid);
            printf("send to inputModulePid = %d\n", inputModulePid);
        }
        else if (checkButtonClick(stopButton, (float)mouseX, (float)mouseY))
        {
            // 녹음 중지 코드
            printf("\nStop button clicked\n");
            recodeStop(inputModulePid);
        }
        else if (checkButtonClick(openButton, (float)mouseX, (float)mouseY))
        {
            // 녹음 중지 코드
            printf("Open button clicked\n");
            inputModulePid = openPCM();
            printf("inputModulePid = %d\n", inputModulePid);
        }
        else if (checkButtonClick(modulationButton, (float)mouseX, (float)mouseY))
        {
            printf("modulation button clicked\n");
            pid_t outputModulePid = getPidByName("outputModule");
            kill(outputModulePid, SIGUSR3);
            printf("send to outputModulePid = %d\n", outputModulePid);
        }
        else if (checkButtonClick(volumeInputBox, (float)mouseX, (float)mouseY))
        {
            // 볼륨 입력 박스 클릭 처리
            printf("Volume input box clicked\n");
            isVolumeInputActive = true;
        }
        else if (checkButtonClick(reset, (float)mouseX, (float)mouseY))
        {
            // 볼륨 초기화
            volumeLevel = 0;

            printf("Volume reset\n");
            printf("Volume level: %d\n", volumeLevel);
        }
        else
        {
            isVolumeInputActive = false;
        }
    }
}

uint32_t *main(void *arg)
{
    int fake_argc = 1;
    char *fake_argv[] = {"program", NULL};
    glutInit(&fake_argc, fake_argv);
    if (!glfwInit())
    {
        // mod->status = MODULE_MODE_FAIL;
    }

    GLFWwindow *window = glfwCreateWindow(800, 600, "Audio Recorder", NULL, NULL);
    if (!window)
    {
        // mod->status = MODULE_MODE_FAIL;

        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetWindowCloseCallback(window, window_close_callback); // 창 닫기 콜백 설정
    glfwSetCharCallback(window, text_input_callback);

    // 초기 뷰포트 설정
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    framebuffer_size_callback(window, width, height);

    while (!glfwWindowShouldClose(window) && !shouldTerminate)
    {
        glClear(GL_COLOR_BUFFER_BIT);

        // 버튼 그리기
        glColor3f(1.0f, 0.0f, 0.0f);
        drawButton(recordButton, "Record");

        glColor3f(0.0f, 1.0f, 0.0f);
        drawButton(stopButton, "Stop");

        glColor3f(0.0f, 0.0f, 1.0f);
        drawButton(openButton, "open");

        glColor3f(1.0f, 0.0f, 1.0f);
        drawButton(modulationButton, "modulation");
        char volumeText[256];

        snprintf(volumeText, sizeof(volumeText), "Enter volume: %d", volumeLevel);

        drawTextBox(50.0f, 500.0f, 200.0f, 30.0f, "Moddify volume:");
        drawTextBox(600.0f, 500.0f, 60.0f, 30.0f, "reset:");

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    printf("Draw module is terminated\n");
    // process 종료시키는 함수
    exit(0);
    return 0;
}
