#include "drawModule.h"
#include <messageModule.h>
extern mqd_t centerQueueId;
mqd_t drawQueueId;
Button recordButton = {100.0f, 100.0f, 100.0f, 50.0f};
Button stopButton = {300.0f, 100.0f, 100.0f, 50.0f};
bool isRecording = false;
volatile int shouldTerminate = 0;
Message *rx = NULL;
Message *tx = NULL;

int checkButtonClick(Button button, float mouseX, float mouseY)
{
    return (mouseX > button.x && mouseX < button.x + button.width &&
            mouseY > button.y && mouseY < button.y + button.height);
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
    // drawText(button.x + button.width / 2, button.y + button.height / 2, label);
}
void window_close_callback(GLFWwindow *window)
{
    shouldTerminate = 1;
    glfwSetWindowShouldClose(window, GLFW_TRUE);
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
            isRecording = true;
            tx->cmd = CMD_SOUND_MODULE_RECODE_START;
            tx->from = MODULE_DRAW;
            tx->to = MODULE_SOUND;
            routeMessages(MODULE_SOUND, MODULE_DRAW, tx);
        }
        else if (checkButtonClick(stopButton, (float)mouseX, (float)mouseY))
        {
            // 녹음 중지 코드
            printf("Stop button clicked\n");
            isRecording = false;
            tx->cmd = CMD_SOUND_MODULE_RECODE_STOP;
            tx->from = MODULE_DRAW;
            tx->to = MODULE_SOUND;
            routeMessages(MODULE_SOUND, MODULE_DRAW, tx);
        }
    }
}

uint32_t *drawModule(void *arg)
{
    drawQueueId = initMessageQueue(QUEUE_DRAW_MODULE);

    rx = malloc(sizeof(Message));
    tx = malloc(sizeof(Message));

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

    // 초기 뷰포트 설정
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    framebuffer_size_callback(window, width, height);

    tx->cmd = CMD_MODDULE_CREATE_DONE;
    tx->from = MODULE_DRAW;
    tx->to = MODULE_MAIN;
    strcpy(tx->data, "Draw module is created");

    while (!glfwWindowShouldClose(window) && !shouldTerminate)
    {
        glClear(GL_COLOR_BUFFER_BIT);

        // 버튼 그리기
        glColor3f(1.0f, 0.0f, 0.0f);
        drawButton(recordButton, "Record");

        glColor3f(0.0f, 1.0f, 0.0f);
        drawButton(stopButton, "Stop");

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // routeMessages(MODULE_MAIN, MODULE_DRAW, tx);
    while (true)
    {

        getMessage(drawQueueId, rx);

        switch (rx->cmd)
        {

        case CMD_MODDULE_CREATE:
            printf("Draw module is created\n");
            rx->to = MODULE_MAIN;
            rx->cmd = CMD_MODDULE_CREATE_DONE;
            break;

            // case CMD_MODDULE_DESTROY:

            //     glfwDestroyWindow(window);
            //     glfwTerminate();
            //     rx->to = MODULE_MAIN;
            //     rx->cmd = CMD_MODDULE_DESTROY_DOEN;
            //     routeMessages(centerQueueId, "main", "draw", tx);
            //     break;

        default:
            break;
        }
    }

    printf("Draw module is terminated\n");
    return 0;
}
