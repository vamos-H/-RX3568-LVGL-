#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include "chess.h"
#include "xiangqi.h"





int main(int argc, char **argv)
{
    lv_init();

    /*Linux frame buffer device init*/
    lv_display_t * disp = lv_linux_fbdev_create();
    lv_linux_fbdev_set_file(disp, "/dev/fb0");
    lv_indev_t * indev = lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event6");

    NetworkConfig net_config;
    memset(&net_config, 0, sizeof(NetworkConfig));
    if (argc == 3) {
        // 从命令行参数获取IP和端口
        strncpy(net_config.ip, argv[1], sizeof(net_config.ip) - 1);
        strncpy(net_config.port, argv[2], sizeof(net_config.port) - 1);
    } else {
        perror("Usage: ./chess <IP> <Port>\n");
        return -1;
    }
    
    game_screen();
    main_screen(net_config);

    /*Handle LVGL tasks*/
    while(1) {
        lv_timer_handler();
        usleep(5000);
    }

    return 0;
}


