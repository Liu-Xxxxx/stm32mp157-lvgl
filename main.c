#include "lvgl/lvgl.h"
#include "lv_drivers/display/fbdev.h"
// #include "lv_demos/lv_demo.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include "lv_drivers/indev/libinput_drv.h"
#include "squareLine/ui.h"
#include "user/mqtt_iot.h"

#define DISP_BUF_SIZE (128 * 1024)

bool isConnected = false;
pthread_t discon_t;

static void *mqtt_disconnect_t(void *argv)
{
    while (1)
    {
        char ch;
        ch = getchar();
        if (ch == 'Q' || ch == 'q')
        {
            printf("Try to exit mqtt task\n");
            if (mqtt_disconnect() == EXIT_SUCCESS)
                break;
        }
    }
    isConnected = false;
    pthread_exit(&discon_t); // 退出线程
    return NULL;
}

int main(void)
{
    /*LittlevGL init*/
    lv_init();
    /*Linux frame buffer device init*/
    fbdev_init();

    /*A small buffer for LittlevGL to draw the screen's content*/
    static lv_color_t buf[DISP_BUF_SIZE];

    /*Initialize a descriptor for the buffer*/
    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);

    /*Initialize and register a display driver*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &disp_buf;
    disp_drv.flush_cb = fbdev_flush;
    disp_drv.hor_res = 1024;
    disp_drv.ver_res = 600;
    lv_disp_drv_register(&disp_drv);

    libinput_init();
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = libinput_read;
    lv_indev_drv_register(&indev_drv);

    /*Create a Demo*/
    // lv_demo_widgets();
    ui_init();

    // 成功建立客户端和服务器的连接且订阅主题后才创建断开连接的线程
    if (mqtt_iot() == 0)
    {
        isConnected = true;
        pthread_create(&discon_t, 0, mqtt_disconnect_t, NULL);
    }
    /*Handle LitlevGL tasks (tickless mode)*/
    while (1)
    {
        lv_task_handler();
        usleep(5000);
    }

    return 0;
}

/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if (start_ms == 0)
    {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}
