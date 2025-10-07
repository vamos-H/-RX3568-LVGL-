#include "network_timer.h"
#include "chess.h"
#include "network.h"
#include "lvgl.h"
#include <stdio.h>

static lv_timer_t* connection_check_timer = NULL;

// 连接检查回调函数
static void connection_check_cb(lv_timer_t* timer) {
    extern NetworkState network;
    extern lv_obj_t* wait_ui;
    extern lv_obj_t* game_ui;

    if(network.room_id != -1) {
        // 连接成功，停止计时器
        stop_connection_check_timer();
        
        // 清除等待界面
        if(wait_ui) {
            lv_obj_clean(wait_ui);
            lv_obj_del(wait_ui);
            wait_ui = NULL;
        }

        // 初始化游戏界面
        qizi_init();
        game_screen();
        lv_obj_remove_flag(game_ui, LV_OBJ_FLAG_HIDDEN);
        
        printf("Connection established, room_id: %d\n", network.room_id);
    }
}

// 启动连接检查计时器
void start_connection_check_timer(void) {
    if(!connection_check_timer) {
        printf("Starting connection check timer...\n");
        connection_check_timer = lv_timer_create(connection_check_cb, 100, NULL);  // 每100ms检查一次
    }
}

// 停止连接检查计时器
void stop_connection_check_timer(void) {
    if(connection_check_timer) {
        lv_timer_del(connection_check_timer);
        connection_check_timer = NULL;
    }
}