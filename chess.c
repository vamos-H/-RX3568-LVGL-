
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include "chess.h"
#include <string.h>
#include "network_timer.h"

lv_obj_t *main_ui = NULL;
lv_obj_t *game_ui = NULL;
lv_obj_t *net_ui = NULL;
lv_obj_t *wait_ui = NULL;

NetworkState network;

lv_obj_t *chi_image_obj = NULL;       // 全局指针，指向“吃子”图片对象
lv_timer_t *chi_hide_timer = NULL; 

MoveHistory move_history[500];  // 移动历史记录
int move_count = 0;             // 当前移动步数
int current_move = 0;           // 当前显示的移动步数

lv_obj_t *suff_btn = NULL;

lv_obj_t *undo_btn = NULL;
lv_obj_t *redo_btn = NULL;
lv_obj_t *undo_label = NULL;
lv_obj_t *redo_label = NULL;

// 全局变量，记录悔棋状态
static int undo_requested = 0;  // 是否已请求悔棋
static int undo_responded = 0;  // 是否已回应悔棋请求
static int undo_accepted = 0;   // 对方是否同意悔棋
lv_obj_t *wait_msg = NULL;  // 等待消息框
lv_obj_t *msg_acc = NULL;   // 悔棋成功消息框
lv_obj_t *msg_rej = NULL;   // 悔棋拒绝消息框
lv_obj_t *msg_timeout = NULL; // 悔棋超时消息框
static lv_obj_t *undo_dialog = NULL;
static lv_timer_t *undo_timeout_timer = NULL;

void display_chi_image(int captured_camp) 
{
    const char* image_src;
    if (captured_camp == RED) 
    {
        image_src = "A:./pic/chi1.png"; // 吃红棋显示 chi1.png
    } 
    else 
    { // BLACK
        image_src = "A:./pic/chi2.png"; // 吃黑棋显示 chi2.png
    }

    if (chi_image_obj == NULL) 
    {
        chi_image_obj = lv_image_create(game_ui);
        lv_obj_align(chi_image_obj, LV_ALIGN_CENTER, 0, 0); // 显示在中心
    } 
    else 
    {
        lv_obj_clear_flag(chi_image_obj, LV_OBJ_FLAG_HIDDEN); // 如果已存在，则使其可见
    }
    lv_image_set_src(chi_image_obj, image_src); // 设置图片源
}

// 延迟隐藏图片的回调函数
static void hide_chi_image_cb(lv_timer_t *timer) 
{
    if (chi_image_obj != NULL) 
    {
        lv_obj_add_flag(chi_image_obj, LV_OBJ_FLAG_HIDDEN);
    }
    
    // 删除定时器
    lv_timer_del(timer);
    chi_hide_timer = NULL;
}

// 触发显示和延迟隐藏图片效果的函数
void show_chi_effect(int captured_camp) 
{
    display_chi_image(captured_camp);
    
    // 如果已有定时器，先删除
    if (chi_hide_timer != NULL) {
        lv_timer_del(chi_hide_timer);
        chi_hide_timer = NULL;
    }
    
    // 创建新定时器
    chi_hide_timer = lv_timer_create(hide_chi_image_cb, 1000, NULL);
    lv_timer_set_repeat_count(chi_hide_timer, 1);
}


void init_screen_click() 
{
    lv_obj_t * screen = lv_scr_act();
    lv_obj_add_flag(screen, LV_OBJ_FLAG_CLICKABLE); // 使屏幕可点击
    lv_obj_add_event_cb(screen, qizi_event_handler, LV_EVENT_CLICKED, NULL);
}

void button_s_callback(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED)
    {
        player = RED;
        qizi_init();
        game_screen();
        lv_obj_add_flag(main_ui,LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(game_ui,LV_OBJ_FLAG_HIDDEN);
    }
}


void main_screen(NetworkConfig net_config)
{
    
    main_ui = lv_obj_create(lv_screen_active());
    lv_obj_set_size(main_ui,1024,600);
    lv_obj_align(main_ui,LV_ALIGN_CENTER,0,0);

    static lv_style_t main_style;
    lv_style_init(&main_style);
    lv_style_set_pad_all(&main_style, 0); //内边距为0
    lv_style_set_border_width(&main_style, 0); //边框的宽度为0，移除边框
    lv_style_set_bg_color(&main_style, lv_color_hex(0x00E3AD75));
    lv_obj_add_style(main_ui, &main_style, 0); //样式添加到窗口控件中

    //打印棋盘图片
    lv_obj_t * pic1 = lv_image_create(main_ui);
    lv_obj_align(pic1,LV_ALIGN_CENTER,0,0);
    lv_image_set_src(pic1,"A:./pic/qipan1.bmp");

    //创建开始按键
    lv_obj_t* button_s = lv_button_create(main_ui);
    lv_obj_set_size(button_s,81,230);
    lv_obj_align(button_s,LV_ALIGN_CENTER,-100,0);

    //设置按键A的样式
    static lv_style_t buttonA_style;
    lv_style_init(&buttonA_style);
    lv_style_set_pad_all(&buttonA_style, 0); //内边距为0
    lv_style_set_border_width(&buttonA_style, 0);
    lv_style_set_bg_color(&buttonA_style, lv_color_hex(0x00E3AD75));
    lv_obj_add_style(button_s, &buttonA_style, 0);

    //在按键上创建一个图片控件指定显示图片的路径
    lv_obj_t *buttonA_image = lv_image_create(button_s);
    lv_image_set_src(buttonA_image, "A:./pic/start_game.png");

    lv_obj_add_event_cb(button_s, button_s_callback, LV_EVENT_CLICKED, NULL);

    // 创建网络对战按钮
    lv_obj_t* button_net = lv_button_create(main_ui);
    lv_obj_set_size(button_net, 81, 230);
    lv_obj_align(button_net, LV_ALIGN_CENTER, 100, 0);
    
    // 设置按钮样式
    static lv_style_t buttonNet_style;
    lv_style_init(&buttonNet_style);
    lv_style_set_pad_all(&buttonNet_style, 0);
    lv_style_set_border_width(&buttonNet_style, 0);
    lv_style_set_bg_color(&buttonNet_style, lv_color_hex(0x00E3AD75));
    lv_obj_add_style(button_net, &buttonNet_style, 0);
    
    // 在按键上创建一个图片控件
    lv_obj_t *buttonNet_image = lv_image_create(button_net);
    lv_image_set_src(buttonNet_image, "A:./pic/network_game.png");
    
    // 添加网络对战回调
    lv_obj_add_event_cb(button_net, button_net_callback, LV_EVENT_CLICKED, &net_config);


    
}

void button_q_callback(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED)
    {
        if(network.mode == NETWORK_MODE_NONE || network.connected == 0) {
            // 如果当前没有网络连接，直接返回主菜单
            lv_obj_add_flag(game_ui,LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(main_ui,LV_OBJ_FLAG_HIDDEN);
            if(net_ui != NULL) {        // 如果网络设置界面存在，删除它
                lv_obj_del(net_ui);
                net_ui = NULL;
            }
            network.mode = NETWORK_MODE_NONE;
            network.connected = 0;
            player = RED; // 重置玩家为红方
            current_move = 0; // 重置当前移动步数
            move_count = 0;   // 重置总移动步数
            qizi_init(); // 重置棋盘
            printf("返回主菜单\n");
            return;
        }
        else {
            if(net_ui != NULL) {        // 如果网络设置界面存在，删除它
                lv_obj_del(net_ui);
                net_ui = NULL;
            }

            // 如果有网络连接，先断开连接
            network_cleanup(&network);
        
            lv_obj_add_flag(game_ui,LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(main_ui,LV_OBJ_FLAG_HIDDEN);
            if(net_ui != NULL) {        // 如果网络设置界面存在，删除它
                lv_obj_del(net_ui);
                net_ui = NULL;
            }
            network.mode = NETWORK_MODE_NONE;
            network.connected = 0;
            network.room_id = -1;
            //memset(&network, 0, sizeof(NetworkState)); // 重置网络状态
            player = RED; // 重置玩家为红方
            current_move = 0; // 重置当前移动步数
            move_count = 0;   // 重置总移动步数
            qizi_init(); // 重置棋盘
            printf("返回主菜单\n");
            return;
        }

    }
}

// 获取LVGL棋子对象对应的Qizi结构体指针
Qizi* get_qizi_by_lv_obj(lv_obj_t* obj) 
{
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 9; ++j) {
            if (chessboard[i][j] != NULL && chessboard[i][j]->live && chessboard[i][j]->pic == obj) {
                return chessboard[i][j];
            }
        }
    }
    return NULL;
}


// lv_obj_t *countdown_label = NULL;
// lv_timer_t *countdown_timer = NULL;
// int countdown_seconds = 60; // 60秒倒计时

// // 创建倒计时标签
// void create_countdown_label(void) {
//     if (countdown_label == NULL) {
//         countdown_label = lv_label_create(game_ui);
//         lv_obj_set_style_text_font(countdown_label, &lv_font_montserrat_24, 0);
//         lv_obj_set_style_text_color(countdown_label, lv_color_hex(0xFF0000), 0);
//         lv_obj_align(countdown_label, LV_ALIGN_TOP_RIGHT, -200, 100);
        
//         // 初始显示
//         static char countdown_text[20];
//         snprintf(countdown_text, sizeof(countdown_text), "倒计时: %02d", countdown_seconds);
//         lv_label_set_text(countdown_label, countdown_text);
//     }
// }

// // 更新倒计时的回调函数
// void update_countdown_cb(lv_timer_t *timer) {
//     if (countdown_seconds > 0) {
//         countdown_seconds--;
        
//         // 更新显示
//         static char countdown_text[20];
//         snprintf(countdown_text, sizeof(countdown_text), "倒计时: %02d", countdown_seconds);
//         lv_label_set_text(countdown_label, countdown_text);
        
//         // 当倒计时少于10秒时改变颜色为红色
//         if (countdown_seconds <= 10) {
//             lv_obj_set_style_text_color(countdown_label, lv_color_hex(0xFF0000), 0);
//         } else {
//             lv_obj_set_style_text_color(countdown_label, lv_color_hex(0x000000), 0);
//         }
//     } else {
//         // 倒计时结束，游戏结束
//         stop_countdown();
        
//         // 显示游戏结束信息
//         lv_obj_t *game_over_label = lv_label_create(game_ui);
//         lv_obj_set_style_text_font(game_over_label, &lv_font_montserrat_32, 0);
//         lv_obj_set_style_text_color(game_over_label, lv_color_hex(0xFF0000), 0);
//         lv_label_set_text(game_over_label, player == RED ? "黑方胜利!" : "红方胜利!");
//         lv_obj_align(game_over_label, LV_ALIGN_CENTER, 0, 0);
        
//         // 添加一个重新开始按钮
//         lv_obj_t *restart_btn = lv_btn_create(game_ui);
//         lv_obj_set_size(restart_btn, 120, 50);
//         lv_obj_align(restart_btn, LV_ALIGN_CENTER, 0, 60);
//         lv_obj_t *restart_label = lv_label_create(restart_btn);
//         lv_label_set_text(restart_label, "重新开始");
//         lv_obj_center(restart_label);
        
//         // 重新开始按钮的回调
//         lv_obj_add_event_cb(restart_btn, button_s_callback, LV_EVENT_CLICKED, NULL);
        
//         printf("时间到! %s胜利\n", player == RED ? "黑方" : "红方");
//     }
// }

// // 重置倒计时
// void reset_countdown(void) {
//     countdown_seconds = 60;
    
//     // 更新显示
//     if (countdown_label) {
//         static char countdown_text[20];
//         snprintf(countdown_text, sizeof(countdown_text), "倒计时: %02d", countdown_seconds);
//         lv_label_set_text(countdown_label, countdown_text);
//         lv_obj_set_style_text_color(countdown_label, lv_color_hex(0x000000), 0);
//     }
// }

// // 停止倒计时
// void stop_countdown(void) {
//     if (countdown_timer) {
//         lv_timer_del(countdown_timer);
//         countdown_timer = NULL;
//     }
// }

// // 开始倒计时
// void start_countdown(void) {
//     stop_countdown(); // 确保没有其他计时器在运行
    
//     countdown_timer = lv_timer_create(update_countdown_cb, 1000, NULL);
//     lv_timer_set_repeat_count(countdown_timer,10000);
// }


int movement_che(Qizi *qizi,int target_x,int target_y)
{
    if(target_x!=qizi->x && target_y!=qizi->y)  //目标位置不在同一行或同一列直接返回-1
    {
        printf("车走法错误1.\n");
        return -1;
    }
    else if(target_x == qizi->x)
    {
        if(qizi->y<target_y)
        {
            for(int i=qizi->y+1;i<target_y;i++) //到目标位置之间不存在棋子
            {
                if(chessboard[qizi->x][i]!=NULL)
                {
                    printf("车走法错误2.\n");
                    return -1;
                }
            }
        }
        else
        {
            for(int i=qizi->y-1;i>target_y;i--)
            {
                if(chessboard[qizi->x][i]!=NULL)
                {
                    printf("车走法错误3.\n");
                    return -1;
                }

            }
        }
    }
    else
    {
        if(qizi->x < target_x)
        {
            for(int i=qizi->x+1;i<target_x;i++)
            {
                if(chessboard[i][qizi->y]!=NULL)
                {
                    printf("车走法错误4.\n");
                    return -1;
                }

            }
        }
        else
        {
            for(int i=qizi->x-1;i>target_x;i--)
            {
                if(chessboard[i][qizi->y]!=NULL)
                {
                    printf("车走法错误5.\n");
                    return -1;
                }

            }
        }
    }
    return 1;
}

int movement_ma(Qizi *qizi,int target_x,int target_y)
{
    int num = (qizi->x - target_x)*(qizi->x - target_x) + (qizi->y - target_y)*(qizi->y - target_y);
    if(num!=5)
    {
        printf("马走法错误\n");
        return -1;
    }

    if((abs(qizi->x-target_x))>(abs(qizi->y-target_y)))
    {
        if(qizi->x > target_x)
        {
            if(chessboard[qizi->x-1][qizi->y]!=NULL)
            {
                printf("路径被阻断1\n");
                return -1;
            }
        }
        else
        {
            if(chessboard[qizi->x+1][qizi->y]!=NULL)
            {
                printf("路径被阻断2\n");
                return -1;
            }
        }

    }
    else
    {
        if(qizi->y>target_y)
        {
            if(chessboard[qizi->x][qizi->y-1]!=NULL)
            {
                printf("路径被阻断3\n");
                return -1;
            }
        }
        else
        {
            if(chessboard[qizi->x][qizi->y+1]!=NULL)
            {
                printf("路径被阻断4\n");
                return -1;
            }
        }
    }
    return 1;
}

int movement_xiang(Qizi *qizi,int target_x,int target_y)
{

    int num = (qizi->x - target_x)*(qizi->x - target_x) + (qizi->y - target_y)*(qizi->y - target_y);
    if(num!=8)
    {
        printf("象走法错误\n");
        return -1;
    }

    if(qizi->camp==RED)
    {
        if(target_x<5)
        {
            printf("象不能过河\n");
            return -1;
        }
    }
    else
    {
        if(target_x>4)
        {
            printf("象不能过河\n");
            return -1;
        }
    }

    if(qizi->y >target_y)
    {
        if(qizi->x >target_x)
        {
            if(chessboard[qizi->x-1][qizi->y-1]!=NULL)
            {
                printf("路径被阻断1\n");
                return -1;
            }
        }
        else
        {
             if(chessboard[qizi->x+1][qizi->y-1]!=NULL)
            {
                printf("路径被阻断2\n");
                return -1;
            }
        }
    }
    else
    {
        if(qizi->x > target_x)
        {
            if(chessboard[qizi->x-1][qizi->y+1]!=NULL)
            {
                printf("路径被阻断3\n");
                return -1;
            }
        }
        else
        {
            if(chessboard[qizi->x+1][qizi->y+1]!=NULL)
            {
                printf("路径被阻断4\n");
                return -1;
            }
        }
    }
    return 1;
}

int movement_shi(Qizi *qizi,int target_x,int target_y)
{
    int num = (qizi->x - target_x)*(qizi->x - target_x) + (qizi->y - target_y)*(qizi->y - target_y);
    if(num!=2)
    {
        printf("士走法错误\n");
        return -1;
    }
    if(chessboard[qizi->x][qizi->y]->camp==RED)
    {
        if(target_y>5||target_y<3)
        {
            printf("士不能走远1\n");
            return -1;
        }
        if(target_x<7)
        {
            printf("士不能走远2\n");
            return -1;
        }
    }
    else
    {
        if(target_y>5||target_y<3)
        {
            printf("士不能走远3\n");
            return -1;
        }
        if(target_x>2)
        {
            printf("士不能走远4\n");
            return -1;
        }
    }
    return 1;
}

int movement_jiang(Qizi *qizi,int target_x,int target_y)
{
    int n =0;
    if(red_s.y==black_j.y)
    {

        for(int i=black_j.x+1;i<red_s.x;i++)
        {
            if(chessboard[i][red_s.y]!=NULL)
            {
                n++;
            }
        }
        if(0==n)
        {
            if(player==RED)
            {
                if(target_x==black_j.x && target_y==black_j.y)
                {
                    return 1;
                }
            }
            else
            {
                if(target_x==red_s.x && target_y==red_s.y)
                {
                    return 1;
                }
            }
        }
    }


    int num = (qizi->x - target_x)*(qizi->x - target_x) + (qizi->y - target_y)*(qizi->y - target_y);
    if(num!=1)
    {
        printf("帅走法错误\n");
        return -1;
    }
    if(qizi->camp==RED)
    {
        if(target_y>5||target_y<3)
        {
            printf("帅不能走远1\n");
            return -1;
        }
        if(target_x<7)
        {
            printf("帅不能走远2\n");
            return -1;
        }
    }
    else
    {
        if(target_y>5||target_y<3)
        {
            printf("帅不能走远3\n");
            return -1;
        }
        if(target_x>2)
        {
            printf("帅不能走远4\n");
            return -1;
        }
    }
    return 1;
}

int movement_bing(Qizi *qizi,int target_x,int target_y)
{
    int num = (qizi->x - target_x)*(qizi->x - target_x) + (qizi->y - target_y)*(qizi->y - target_y);
    if(num!=1)
    {
        printf("兵走法错误\n");
        return -1;
    }
    if(qizi->camp==RED)
    {
        if(target_x>qizi->x)
        {
            printf("兵不能回头\n");
            return -1;
        }
        if(target_x>4)
        {
            if(target_y!=qizi->y)
            {
                printf("过河前不能左右跑\n");
                return -1;
            }
        }
    }
    else 
    {
        if(target_x<qizi->x)
        {
            printf("兵不能回头\n");
            return -1;
        }
        if(target_x<5)
        {
            if(target_y!=qizi->y)
            {
                printf("过河前不能左右跑\n");
                return -1;
            }
        }
    }
    return 1;
}

int movement_pao(Qizi *qizi,int target_x,int target_y)
{
    int g=0;
    if(target_x!=qizi->x && target_y!=qizi->y)  //目标位置不在同一行或同一列直接返回-1
    {
        printf("炮走法错误1.\n");
        return -1;
    }
    else if(target_x==qizi->x)  //在y轴上走
    {
        if(qizi->y<target_y)
        {
            
            for(int i=qizi->y+1;i<=target_y;i++) //到目标位置之间不存在棋子
            {
                if(chessboard[qizi->x][i]!=NULL)
                {
                    g++;
                }

            }
            if(g==0)
            {
                return 1;
            }
            else if(g==2)
            {
                return 1;
            }
            else
            {
                printf("炮走法错误1\n");
                return -1;
            }
        }
        else
        {
            for(int i=qizi->y-1;i>=target_y;i--)
            {
                if(chessboard[qizi->x][i]!=NULL)
                {
                    g++;
                }

            }
            if(g==0)
            {
                return 1;
            }
            else if(g==2)
            {
                return 1;
            }
            else
            {
                printf("炮走法错误2\n");
                return -1;
            }

        }
    }
    else if(target_y==qizi->y)
    {
        if(qizi->x<target_x)
        {
            for(int i=qizi->x+1;i<=target_x;i++)
            {
                if(chessboard[i][qizi->y]!=NULL)
                {
                    g++;
                }
            }
            
            if(g==0)
            {
                return 1;
            }
            else if(g==2)
            {
                return 1;
            }
            else
            {
                printf("炮走法错误3\n");
                return -1;
            }
        }
        else
        {

            for(int i=qizi->x-1;i>=target_x;i--)
            {
                if(chessboard[i][qizi->y]!=NULL)
                {
                    g++;
                }

            }
            if(g==0)
            {
                return 1;
            }
            else if(g==2)
            {
                return 1;
            }
            else
            {
                printf("炮走法错误4\n");
                return -1;
            }

        }

    }
    return 1;
}



static void anim_completed_cb(lv_anim_t * anim) 
{
    Qizi *qizi = anim->user_data;
    if (qizi) 
    {
        // 动画完成后可以执行一些操作，比如更新状态
        printf("动画完成: 棋子移动到 (%d, %d)\n", qizi->x, qizi->y);
    }
}

// 设置棋子位置的动画函数
static void set_qizi_pos_anim(void * var, int32_t value_x, int32_t value_y) 
{
    lv_obj_t * obj = (lv_obj_t *)var;
    lv_obj_set_pos(obj, value_x, value_y);
}

void animate_qizi_move(Qizi* qizi, int target_x, int target_y) 
{
    if (qizi == NULL || qizi->pic == NULL) return;
    
    // 计算起始和结束位置（屏幕坐标）
    int start_x = lv_obj_get_x(qizi->pic);
    int start_y = lv_obj_get_y(qizi->pic);
    
    // 计算目标屏幕坐标
    int end_x = 182 + 36 - 30 + 65 * target_x;
    int end_y = 45 - 30 + 65 * target_y;
    
    // 创建X轴动画
    lv_anim_t anim_x;
    lv_anim_init(&anim_x);
    lv_anim_set_var(&anim_x, qizi->pic);
    lv_anim_set_values(&anim_x, start_x, end_x);
    lv_anim_set_time(&anim_x, 300);  // 动画时长300ms
    lv_anim_set_exec_cb(&anim_x, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_path_cb(&anim_x, lv_anim_path_ease_out);  // 使用缓动效果
    lv_anim_set_ready_cb(&anim_x, anim_completed_cb);
    lv_anim_set_user_data(&anim_x, qizi);
    lv_anim_start(&anim_x);
    
    // 创建Y轴动画
    lv_anim_t anim_y;
    lv_anim_init(&anim_y);
    lv_anim_set_var(&anim_y, qizi->pic);
    lv_anim_set_values(&anim_y, start_y, end_y);
    lv_anim_set_time(&anim_y, 300);
    lv_anim_set_exec_cb(&anim_y, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_path_cb(&anim_y, lv_anim_path_ease_out);
    lv_anim_start(&anim_y);
}


// 棋子移动函数
void move_qizi(Qizi* qizi, int target_x, int target_y)
{
    printf("移动内部坐标: target_x=%d target_y=%d\n",target_x,target_y);
    printf("移动内部坐标: qizi_x=%d qizi_y=%d\n",qizi->x,qizi->y);
    printf("准备移动\n");
    int flag = 1;
    if(qizi==NULL)
    {
        return ;
    }

    switch(qizi->type)
    {
        case QZ_C:
            flag = movement_che(qizi,target_x,target_y);
            break;
        case QZ_M:
            flag = movement_ma(qizi,target_x,target_y);
            break;
        case QZ_X:
            flag = movement_xiang(qizi,target_x,target_y);
            break;
        case QZ_S:
            flag = movement_shi(qizi,target_x,target_y);
            break;
        case QZ_J:
            flag = movement_jiang(qizi,target_x,target_y);
            break;
        case QZ_B:
            flag = movement_bing(qizi,target_x,target_y);
            break;
        case QZ_P:
            flag = movement_pao(qizi,target_x,target_y);
            break;
    }

    if(-1==flag)
    {
        qizi->selected = false; // 取消选中状态
        selected_qizi = NULL;   // 清空全局选中棋子指针
        return ;
    }



    int from_x = qizi->x;
    int from_y = qizi->y;
    Qizi* captured = NULL;

    if ((network.connected && network.mode == NETWORK_MODE_SERVER && player == RED)||(network.connected && network.mode == NETWORK_MODE_CLIENT && player == BLACK)) {
        
        NetworkMessage msg;
        msg.type = MSG_MOVE;
        msg.data[0] = from_x;
        msg.data[1] = from_y;
        msg.data[2] = target_x;
        msg.data[3] = target_y;
        
        network_send(&network, &msg);
    }

    if(chessboard[target_x][target_y]!=NULL && chessboard[target_x][target_y]->live==1)
    {
        if(1==flag)
        {
            captured = chessboard[target_x][target_y];
            show_chi_effect(qizi->camp);
            printf("正在删除被吃棋子\n");
            chessboard[target_x][target_y]->live = 0;
            lv_obj_add_flag(chessboard[target_x][target_y]->pic, LV_OBJ_FLAG_HIDDEN); // 隐藏LVGL对象
            printf("删除完毕\n");
        }
    }
    printf("进行移动\n");

    chessboard[qizi->x][qizi->y] = NULL;
    qizi->x = target_x;
    qizi->y = target_y;
    chessboard[target_x][target_y] = qizi;

    animate_qizi_move(qizi, target_x, target_y);
    //lv_obj_align(qizi->pic,LV_ALIGN_TOP_LEFT,182 + 36 - 30 + 65 * qizi->x,45 - 30 + 65 * qizi->y);
    record_move(qizi, from_x, from_y, target_x, target_y, captured);

    qizi->selected = false; // 取消选中状态
    selected_qizi = NULL;   // 清空全局选中棋子指针

    if(red_s.live == 0 || black_j.live == 0)
    {

        printf("game over\n");
        lv_obj_t * game_over = lv_image_create(game_ui);
        lv_obj_align(game_over,LV_ALIGN_CENTER,0,0);
        lv_image_set_src(game_over,"A:./pic/game_over.png");
        return ;

    }

    // 切换玩家回合
    player = (player == RED) ? BLACK : RED;
    printf("当前玩家为: %s\n", (player == RED) ? "RED" : "BLACK");

    // TODO: 添加将军/将死判断
}

int i=0;

void qizi_event_handler(lv_event_t *e)
{
    int target_ture_x = -1,target_ture_y = -1;
    int target_x = -1,target_y = -1;
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* obj = lv_event_get_target(e);

    printf("%d\n",i++);

    if(LV_EVENT_CLICKED==code)
    {
        //Qizi *click_qizi = get_qizi_by_lv_obj(obj);
        lv_indev_t *indev = lv_event_get_indev(e);

        lv_point_t point;
        lv_indev_get_point(indev, &point);
        printf("点击的屏幕坐标为: x=%d, y=%d\n", point.x, point.y);

        target_ture_x = point.x;
        target_ture_y = point.y; 
        target_x = (target_ture_x - 182 - 36 + 30 + 15) / 65;
        target_y = (target_ture_y - 45 + 30 + 15) / 65;
        printf("棋盘坐标为: x=%d, y=%d\n", target_x,target_y);

        if(target_x<0||target_x>9)
        {
            printf("请点击正确的区域/n");
            return ;
        }
        if(target_y<0||target_y>8)
        {
            printf("请点击正确的区域/n");
            return ;
        }

        Qizi *click_qizi = chessboard[target_x][target_y];

        if(click_qizi)  //选中了棋子
        {
            if(selected_qizi == NULL)   //还未选中棋子
            {
                if(network.mode != NETWORK_MODE_NONE) {
                    // 网络对战模式下，检查是否轮到当前玩家
                    if ((network.connected && network.mode == NETWORK_MODE_SERVER && player != RED) ||
                        (network.connected && network.mode == NETWORK_MODE_CLIENT && player != BLACK)) {
                        printf("当前不是你的回合，请等待对方操作。\n");
                        return;
                    }
                }

                if(click_qizi->camp == player)
                {
                    printf("已选择棋子坐标为 x=%d, y=%d\n",click_qizi->x,click_qizi->y);
                    selected_qizi = click_qizi;
                    selected_qizi->selected = true;

                    //添加棋子高亮显示,改变边框样式
                    lv_obj_set_style_border_color(selected_qizi->pic, lv_color_hex(0x66ffff), LV_PART_MAIN); // 红色边框
                    lv_obj_set_style_border_width(selected_qizi->pic, 3, LV_PART_MAIN);

                    //显示可以走向的位置

                }
                else
                {
                    //点击了对面的棋子
                    printf("\n");
                    printf("当前玩家为: %s\n", (player == RED) ? "RED" : "BLACK");
                    printf("棋子阵营为：%s\n", (click_qizi->camp == RED) ? "RED" : "BLACK");
                    printf("请点击自己的棋子。\n");
                    printf("\n");
                }

            }
            else    //已经选中了一枚棋子了
            {
                if(selected_qizi == click_qizi)
                {
                    //移除高亮
                    lv_obj_set_style_border_width(selected_qizi->pic, 0, LV_PART_MAIN); // 移除边框
                    selected_qizi->selected = false;
                    selected_qizi = NULL;

                    //移除可以走向的位置

                }
                else if(selected_qizi->camp == click_qizi->camp)
                {
                    printf("以切换棋子为: x=%d, y=%d\n",click_qizi->x,click_qizi->y);
                    //之前选中的棋子移除高亮
                    lv_obj_set_style_border_width(selected_qizi->pic, 0, LV_PART_MAIN); // 移除边框
                    selected_qizi->selected = false;
                    selected_qizi = click_qizi;
                    selected_qizi->selected = true;

                    //添加棋子高亮显示，例如改变边框样式
                    lv_obj_set_style_border_color(selected_qizi->pic, lv_color_hex(0x66ffff), LV_PART_MAIN); // 红色边框
                    lv_obj_set_style_border_width(selected_qizi->pic, 3, LV_PART_MAIN);

                    //显示可以走向的位置

                }
                else    //点击的对方棋子
                {
                    printf("正常删除中\n");
                    lv_obj_set_style_border_width(selected_qizi->pic, 0, LV_PART_MAIN);
                    move_qizi(selected_qizi,target_x,target_y);
                }
            }
        }
        else    //点击的空白处
        {
            if(selected_qizi==NULL)
            {
                printf("请点击棋子进行移动\n");
                return ;
            }
            else
            {

                printf("往空白处移动\n");
                lv_obj_set_style_border_width(selected_qizi->pic, 0, LV_PART_MAIN);
                move_qizi(selected_qizi,target_x,target_y);
                
            }
        }
    }
}

void button_wait_q_callback(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED)
    {
        if(net_ui != NULL) {        // 如果网络设置界面存在，删除它
            lv_obj_del(net_ui);
            net_ui = NULL;
        }
        lv_obj_add_flag(wait_ui,LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(main_ui,LV_OBJ_FLAG_HIDDEN);
        network.mode = NETWORK_MODE_NONE;
        network.connected = 0;
        player = RED; // 重置玩家为红方
        current_move = 0; // 重置当前移动步数
        move_count = 0;   // 重置总移动步数
        qizi_init(); // 重置棋盘
        printf("返回主菜单\n");
        return;
    }
}

void wait_screen(void)
{
    wait_ui = lv_obj_create(lv_scr_act());
    lv_obj_set_size(wait_ui,1024,600);
    lv_obj_align(wait_ui,LV_ALIGN_CENTER,0,0);

    static lv_style_t wait_style;
    lv_style_init(&wait_style);
    lv_style_set_pad_all(&wait_style, 0); //内边距为0
    lv_style_set_border_width(&wait_style, 0); //边框的宽度为0，移除边框
    lv_style_set_bg_color(&wait_style, lv_color_hex(0x00ffffff));
    lv_obj_add_style(wait_ui, &wait_style, 0); //样式添加到窗口控件中

    // 创建等待文本
    lv_obj_t * wait_label = lv_label_create(wait_ui);
    lv_label_set_text(wait_label, "Waiting for opponent...");
    //lv_obj_align(wait_label, LV_ALIGN_CENTER, 0, -50);
    lv_obj_align(wait_label, LV_ALIGN_TOP_MID, 0, 50);
    
    //打印等待gif图片
    lv_obj_t * pic1 = lv_gif_create(wait_ui);
    lv_obj_align(pic1,LV_ALIGN_CENTER,0,50);
    lv_gif_set_src(pic1,"A:./pic/wait.gif");

    // 在wait_ui上创建退出按钮，而不是在还未创建的game_ui上
    lv_obj_t * button_q = lv_button_create(wait_ui);
    lv_obj_set_size(button_q,100,200);
    lv_obj_set_pos(button_q,0,0);

    lv_obj_add_style(button_q, &wait_style, 0);

    lv_obj_t * pic_quit = lv_image_create(button_q);
    lv_image_set_src(pic_quit,"A:./pic/quit_game.png");

    lv_obj_add_event_cb(button_q, button_wait_q_callback, LV_EVENT_CLICKED, NULL);
    
    // 确保wait_ui是可见的
    lv_obj_clear_flag(wait_ui, LV_OBJ_FLAG_HIDDEN);
    printf("进入等待界面\n");
}

void game_screen(void)
{

    game_ui = lv_obj_create(lv_screen_active());
    lv_obj_set_size(game_ui,1024,600);
    lv_obj_align(game_ui,LV_ALIGN_CENTER,0,0);

    static lv_style_t game_style;
    lv_style_init(&game_style);
    lv_style_set_pad_all(&game_style, 0); //内边距为0
    lv_style_set_border_width(&game_style, 0); //边框的宽度为0，移除边框
    lv_style_set_bg_color(&game_style, lv_color_hex(0x00E3AD75));
    lv_obj_add_style(game_ui, &game_style, 0); //样式添加到窗口控件中

    //打印棋盘图片
    lv_obj_t * pic1 = lv_image_create(game_ui);
    lv_obj_align(pic1,LV_ALIGN_CENTER,0,0);
    lv_image_set_src(pic1,"A:./pic/qipan1.bmp");

    //打印退出按键
    lv_obj_t * button_q = lv_button_create(game_ui);
    lv_obj_set_size(button_q,100,200);
    lv_obj_set_pos(button_q,0,0);

    lv_obj_add_style(button_q, &game_style, 0);

    lv_obj_t * pic_quit = lv_image_create(button_q);
    lv_image_set_src(pic_quit,"A:./pic/quit_game.png");

    //打印棋子

    static lv_style_t chess_style;
    lv_style_init(&chess_style);
    lv_style_set_pad_all(&chess_style, 0); //内边距为0
    lv_style_set_border_width(&chess_style, 0); //边框的宽度为0，移除边框
    lv_style_set_bg_color(&chess_style, lv_color_hex(0x00E3AD75));

    black_c1.pic = lv_image_create(game_ui);
    lv_obj_align(black_c1.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*black_c1.x,45-30+65*black_c1.y);
    lv_image_set_src(black_c1.pic,black_c1.pic_add);
    lv_obj_add_flag(black_c1.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(black_c1.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);

    black_c2.pic = lv_image_create(game_ui);
    lv_obj_align(black_c2.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*black_c2.x,45-30+65*black_c2.y);
    lv_image_set_src(black_c2.pic,black_c2.pic_add);
    lv_obj_add_flag(black_c2.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(black_c2.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);

    black_m1.pic = lv_image_create(game_ui);
    lv_obj_align(black_m1.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*black_m1.x,45-30+65*black_m1.y);
    lv_image_set_src(black_m1.pic,black_m1.pic_add);
    lv_obj_add_flag(black_m1.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(black_m1.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);

    black_m2.pic = lv_image_create(game_ui);
    lv_obj_align(black_m2.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*black_m2.x,45-30+65*black_m2.y);
    lv_image_set_src(black_m2.pic,black_m2.pic_add);
    lv_obj_add_flag(black_m2.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(black_m2.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);

    black_x1.pic = lv_image_create(game_ui);
    lv_obj_align(black_x1.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*black_x1.x,45-30+65*black_x1.y);
    lv_image_set_src(black_x1.pic,black_x1.pic_add);
    lv_obj_add_flag(black_x1.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(black_x1.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);

    black_x2.pic = lv_image_create(game_ui);
    lv_obj_align(black_x2.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*black_x2.x,45-30+65*black_x2.y);
    lv_image_set_src(black_x2.pic,black_x2.pic_add);
    lv_obj_add_flag(black_x2.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(black_x2.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);

    black_s1.pic = lv_image_create(game_ui);
    lv_obj_align(black_s1.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*black_s1.x,45-30+65*black_s1.y);
    lv_image_set_src(black_s1.pic,black_s1.pic_add);
    lv_obj_add_flag(black_s1.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(black_s1.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);

    black_s2.pic = lv_image_create(game_ui);
    lv_obj_align(black_s2.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*black_s2.x,45-30+65*black_s2.y);
    lv_image_set_src(black_s2.pic,black_s2.pic_add);
    lv_obj_add_flag(black_s2.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(black_s2.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);

    black_j.pic = lv_image_create(game_ui);
    lv_obj_align(black_j.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*black_j.x,45-30+65*black_j.y);
    lv_image_set_src(black_j.pic,black_j.pic_add);
    lv_obj_add_flag(black_j.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(black_j.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);

    black_p1.pic = lv_image_create(game_ui);
    lv_obj_align(black_p1.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*black_p1.x,45-30+65*black_p1.y);
    lv_image_set_src(black_p1.pic,black_p1.pic_add);
    lv_obj_add_flag(black_p1.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(black_p1.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);

    black_p2.pic = lv_image_create(game_ui);
    lv_obj_align(black_p2.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*black_p2.x,45-30+65*black_p2.y);
    lv_image_set_src(black_p2.pic,black_p2.pic_add);
    lv_obj_add_flag(black_p2.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(black_p2.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);

    black_b1.pic = lv_image_create(game_ui);
    lv_obj_align(black_b1.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*black_b1.x,45-30+65*black_b1.y);
    lv_image_set_src(black_b1.pic,black_b1.pic_add);
    lv_obj_add_flag(black_b1.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(black_b1.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);

    black_b2.pic = lv_image_create(game_ui);
    lv_obj_align(black_b2.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*black_b2.x,45-30+65*black_b2.y);
    lv_image_set_src(black_b2.pic,black_b2.pic_add);
    lv_obj_add_flag(black_b2.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(black_b2.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);
    
    black_b3.pic = lv_image_create(game_ui);
    lv_obj_align(black_b3.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*black_b3.x,45-30+65*black_b3.y);
    lv_image_set_src(black_b3.pic,black_b3.pic_add);
    lv_obj_add_flag(black_b3.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(black_b3.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);

    black_b4.pic = lv_image_create(game_ui);
    lv_obj_align(black_b4.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*black_b4.x,45-30+65*black_b4.y);
    lv_image_set_src(black_b4.pic,black_b4.pic_add);
    lv_obj_add_flag(black_b4.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(black_b4.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);

    black_b5.pic = lv_image_create(game_ui);
    lv_obj_align(black_b5.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*black_b5.x,45-30+65*black_b5.y);
    lv_image_set_src(black_b5.pic,black_b5.pic_add);
    lv_obj_add_flag(black_b5.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(black_b5.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);




    red_c1.pic = lv_image_create(game_ui);
    lv_obj_align(red_c1.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*red_c1.x,45-30+65*red_c1.y);
    lv_image_set_src(red_c1.pic,red_c1.pic_add);
    lv_obj_add_flag(red_c1.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(red_c1.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);

    red_c2.pic = lv_image_create(game_ui);
    lv_obj_align(red_c2.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*red_c2.x,45-30+65*red_c2.y);
    lv_image_set_src(red_c2.pic,red_c2.pic_add);
    lv_obj_add_flag(red_c2.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(red_c2.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);

    red_m1.pic = lv_image_create(game_ui);
    lv_obj_align(red_m1.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*red_m1.x,45-30+65*red_m1.y);
    lv_image_set_src(red_m1.pic,red_m1.pic_add);
    lv_obj_add_flag(red_m1.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(red_m1.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);

    red_m2.pic = lv_image_create(game_ui);
    lv_obj_align(red_m2.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*red_m2.x,45-30+65*red_m2.y);
    lv_image_set_src(red_m2.pic,red_m2.pic_add);
    lv_obj_add_flag(red_m2.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(red_m2.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);

    red_x1.pic = lv_image_create(game_ui);
    lv_obj_align(red_x1.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*red_x1.x,45-30+65*red_x1.y);
    lv_image_set_src(red_x1.pic,red_x1.pic_add);
    lv_obj_add_flag(red_x1.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(red_x1.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);

    red_x2.pic = lv_image_create(game_ui);
    lv_obj_align(red_x2.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*red_x2.x,45-30+65*red_x2.y);
    lv_image_set_src(red_x2.pic,red_x2.pic_add);
    lv_obj_add_flag(red_x2.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(red_x2.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);

    red_s1.pic = lv_image_create(game_ui);
    lv_obj_align(red_s1.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*red_s1.x,45-30+65*red_s1.y);
    lv_image_set_src(red_s1.pic,red_s1.pic_add);
    lv_obj_add_flag(red_s1.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(red_s1.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);

    red_s2.pic = lv_image_create(game_ui);
    lv_obj_align(red_s2.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*red_s2.x,45-30+65*red_s2.y);
    lv_image_set_src(red_s2.pic,red_s2.pic_add);
    lv_obj_add_flag(red_s2.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(red_s2.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);

    red_s.pic = lv_image_create(game_ui);
    lv_obj_align(red_s.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*red_s.x,45-30+65*red_s.y);
    lv_image_set_src(red_s.pic,red_s.pic_add);
    lv_obj_add_flag(red_s.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(red_s.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);

    red_p1.pic = lv_image_create(game_ui);
    lv_obj_align(red_p1.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*red_p1.x,45-30+65*red_p1.y);
    lv_image_set_src(red_p1.pic,red_p1.pic_add);
    lv_obj_add_flag(red_p1.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(red_p1.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);

    red_p2.pic = lv_image_create(game_ui);
    lv_obj_align(red_p2.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*red_p2.x,45-30+65*red_p2.y);
    lv_image_set_src(red_p2.pic,red_p2.pic_add);
    lv_obj_add_flag(red_p2.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(red_p2.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);

    red_b1.pic = lv_image_create(game_ui);
    lv_obj_align(red_b1.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*red_b1.x,45-30+65*red_b1.y);
    lv_image_set_src(red_b1.pic,red_b1.pic_add);
    lv_obj_add_flag(red_b1.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(red_b1.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);

    red_b2.pic = lv_image_create(game_ui);
    lv_obj_align(red_b2.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*red_b2.x,45-30+65*red_b2.y);
    lv_image_set_src(red_b2.pic,red_b2.pic_add);
    lv_obj_add_flag(red_b2.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(red_b2.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);
    
    red_b3.pic = lv_image_create(game_ui);
    lv_obj_align(red_b3.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*red_b3.x,45-30+65*red_b3.y);
    lv_image_set_src(red_b3.pic,red_b3.pic_add);
    lv_obj_add_flag(red_b3.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(red_b3.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);

    red_b4.pic = lv_image_create(game_ui);
    lv_obj_align(red_b4.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*red_b4.x,45-30+65*red_b4.y);
    lv_image_set_src(red_b4.pic,red_b4.pic_add);
    lv_obj_add_flag(red_b4.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(red_b4.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);

    red_b5.pic = lv_image_create(game_ui);
    lv_obj_align(red_b5.pic,LV_ALIGN_TOP_LEFT,182+36-30+65*red_b5.x,45-30+65*red_b5.y);
    lv_image_set_src(red_b5.pic,red_b5.pic_add);
    lv_obj_add_flag(red_b5.pic, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(red_b5.pic,qizi_event_handler,LV_EVENT_CLICKED,NULL);

    lv_obj_add_event_cb(button_q,button_q_callback,LV_EVENT_CLICKED,NULL);

    lv_obj_add_event_cb(game_ui,qizi_event_handler,LV_EVENT_CLICKED,NULL);



    undo_btn = lv_btn_create(game_ui);
    lv_obj_set_size(undo_btn, 100, 100);
    lv_obj_set_pos(undo_btn, 850, 100);
    lv_obj_add_style(undo_btn, &game_style, 0);
    // undo_label = lv_label_create(undo_btn);
    // lv_label_set_text(undo_label, "regret");
    // lv_obj_center(undo_label);
    undo_label = lv_image_create(undo_btn);
    lv_obj_align(undo_label,LV_ALIGN_CENTER,0,0);
    lv_image_set_src(undo_label,"A:./pic/huiqi.png");
    
    // 悔棋按钮回调
    lv_obj_add_event_cb(undo_btn, undo_btn_event_cb, LV_EVENT_CLICKED, NULL);

    
    // 添加重做按钮
    redo_btn = lv_btn_create(game_ui);
    lv_obj_set_size(redo_btn, 80, 40);
    lv_obj_set_pos(redo_btn, 850, 250);
    redo_label = lv_label_create(redo_btn);
    lv_label_set_text(redo_label, "Start over");
    lv_obj_center(redo_label);
    
    // 重做按钮回调
    lv_obj_add_event_cb(redo_btn, redo_btn_event_cb, LV_EVENT_CLICKED, NULL);

    // 添加认输按钮
    suff_btn = lv_btn_create(game_ui);
    lv_obj_set_size(suff_btn,42,78);
    lv_obj_set_pos(suff_btn,900,450);
    lv_obj_add_style(suff_btn, &game_style, 0);
    lv_obj_t *suff_label = lv_image_create(suff_btn);
    lv_obj_align(suff_label,LV_ALIGN_CENTER,0,0);
    lv_image_set_src(suff_label,"A:./pic/touxiang.png");

    lv_obj_add_event_cb(suff_btn, suff_btn_event_cb, LV_EVENT_CLICKED, NULL);

}

static void suff_btn_event_cb(lv_event_t *e)
{
    if(network.mode == NETWORK_MODE_NONE) {
        // 本地模式下认输
        printf("%s认输，游戏结束！\n", (player == RED) ? "红方" : "黑方");
        if(player == RED)
        {
            lv_obj_t * game_over = lv_image_create(game_ui);
            lv_obj_align(game_over,LV_ALIGN_CENTER,0,0);
            lv_image_set_src(game_over,"A:./pic/red_lost.png");
        }
        else
        {
            lv_obj_t * game_over = lv_image_create(game_ui);
            lv_obj_align(game_over,LV_ALIGN_CENTER,0,0);
            lv_image_set_src(game_over,"A:./pic/black_lost.png");
        }
    }else
    {
        // 发送认输消息
        NetworkMessage msg;
        msg.type = MSG_SUFF;
        network_send(&network, &msg);
        printf("你认输，游戏结束！\n");
        lv_obj_t * game_over = lv_image_create(game_ui);
        lv_obj_align(game_over,LV_ALIGN_CENTER,0,0);
        lv_image_set_src(game_over,"A:./pic/mylost.png");        
    }
}


static void undo_btn_event_cb(lv_event_t * e) 
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) 
    {
        network_undo_move();
    }
}

// 重做按钮回调函数
static void redo_btn_event_cb(lv_event_t * e) 
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) 
    {
        redo_move();
    }
}



void record_move(Qizi* piece, int from_x, int from_y, int to_x, int to_y, Qizi* captured) {
    if (move_count >= 500) 
    {
        printf("移动历史已满，无法记录更多步数\n");
        return;
    }
    
    // 如果当前不是最新步数，删除后面的历史
    if (current_move < move_count) 
    {
        move_count = current_move;
    }
    
    // 记录移动
    move_history[move_count].piece = piece;
    move_history[move_count].from_x = from_x;
    move_history[move_count].from_y = from_y;
    move_history[move_count].to_x = to_x;
    move_history[move_count].to_y = to_y;
    move_history[move_count].captured = captured;
    
    if (captured) 
    {
        move_history[move_count].captured_live = captured->live;
    } 
    else 
    {
        move_history[move_count].captured_live = 1; // 默认存活
    }
    
    move_count++;
    current_move = move_count;
    
    printf("记录移动: %s 从 (%d,%d) 到 (%d,%d)\n", 
           piece->type == QZ_J ? "将" : 
           piece->type == QZ_S ? "士" :
           piece->type == QZ_X ? "相" :
           piece->type == QZ_C ? "车" :
           piece->type == QZ_M ? "马" :
           piece->type == QZ_P ? "炮" : "兵",
           from_x, from_y, to_x, to_y);
}

// 悔棋功能
void undo_move(void) 
{
    if (current_move <= 0) 
    {
        printf("没有可悔棋的步数\n");
        return;
    }

    
    current_move--;
    MoveHistory* move = &move_history[current_move];
    
    // 恢复棋盘状态
    chessboard[move->to_x][move->to_y] = NULL;
    chessboard[move->from_x][move->from_y] = move->piece;
    
    // 恢复棋子位置
    move->piece->x = move->from_x;
    move->piece->y = move->from_y;
    
    // 更新LVGL对象位置
    lv_obj_set_pos(move->piece->pic, 
                  182 + 36 - 30 + 65 * move->from_x, 
                  45 - 30 + 65 * move->from_y);
    
    // 如果有被吃的棋子，恢复它
    if (move->captured != NULL) 
    {
        move->captured->live = move->captured_live;
        chessboard[move->to_x][move->to_y] = move->captured;
        
        // 显示被吃的棋子
        if (move->captured->pic) 
        {
            lv_obj_clear_flag(move->captured->pic, LV_OBJ_FLAG_HIDDEN);
        }
    }
    
    // 切换玩家
    player = (player == RED) ? BLACK : RED;
    
    printf("悔棋成功，当前玩家: %s\n", (player == RED) ? "红方" : "黑方");
    
    // 更新悔棋/重做按钮状态
    update_undo_redo_buttons();
}

// 重做功能
void redo_move(void) {
    if (current_move >= move_count) 
    {
        printf("没有可重做的步数\n");
        return;
    }
    
    MoveHistory* move = &move_history[current_move];
    
    // 执行移动
    chessboard[move->from_x][move->from_y] = NULL;
    chessboard[move->to_x][move->to_y] = move->piece;
    
    // 更新棋子位置
    move->piece->x = move->to_x;
    move->piece->y = move->to_y;
    
    // 更新LVGL对象位置
    lv_obj_set_pos(move->piece->pic, 
                  182 + 36 - 30 + 65 * move->to_x, 
                  45 - 30 + 65 * move->to_y);
    
    // 如果有被吃的棋子，隐藏它
    if (move->captured != NULL) 
    {
        move->captured->live = 0;
        if (move->captured->pic) 
        {
            lv_obj_add_flag(move->captured->pic, LV_OBJ_FLAG_HIDDEN);
        }
    }
    
    current_move++;
    
    // 切换玩家
    player = (player == RED) ? BLACK : RED;
    
    
    printf("重做成功，当前玩家: %s\n", (player == RED) ? "红方" : "黑方");
    
    // 更新悔棋/重做按钮状态
    update_undo_redo_buttons();
}

// 更新悔棋/重做按钮状态
void update_undo_redo_buttons(void) 
{
    if (undo_btn == NULL || redo_btn == NULL) 
    {
        return; // 按钮尚未创建
    }
    
    // 更新悔棋按钮状态
    if (current_move > 0) 
    {
        // 有悔棋步骤可用，启用按钮
        lv_obj_clear_state(undo_btn, LV_STATE_DISABLED);
        lv_obj_set_style_opa(undo_btn, LV_OPA_100, 0);
        if (undo_label) 
        {
            lv_obj_set_style_text_opa(undo_label, LV_OPA_100, 0);
        }
    } 
    else 
    {
        // 没有悔棋步骤可用，禁用按钮
        lv_obj_add_state(undo_btn, LV_STATE_DISABLED);
        lv_obj_set_style_opa(undo_btn, LV_OPA_50, 0);
        if (undo_label) 
        {
            lv_obj_set_style_text_opa(undo_label, LV_OPA_50, 0);
        }
    }
    
    // 更新重做按钮状态
    if (current_move < move_count) 
    {
        // 有重做步骤可用，启用按钮
        lv_obj_clear_state(redo_btn, LV_STATE_DISABLED);
        lv_obj_set_style_opa(redo_btn, LV_OPA_100, 0);
        if (redo_label) 
        {
            lv_obj_set_style_text_opa(redo_label, LV_OPA_100, 0);
        }
    } 
    else 
    {
        // 没有重做步骤可用，禁用按钮
        lv_obj_add_state(redo_btn, LV_STATE_DISABLED);
        lv_obj_set_style_opa(redo_btn, LV_OPA_50, 0);
        if (redo_label) 
        {
            lv_obj_set_style_text_opa(redo_label, LV_OPA_50, 0);
        }
    }
}



void button_net_callback(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    NetworkConfig *config = lv_event_get_user_data(e);
    if(code == LV_EVENT_CLICKED) {
        // 显示网络选择界面
        show_network_selection(config);
    }
}

// 显示网络选择界面
void show_network_selection(NetworkConfig *config) {
    // 创建网络选择界面
    net_ui = lv_obj_create(lv_scr_act());
    lv_obj_set_size(net_ui, 400, 300);
    lv_obj_align(net_ui, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * label = lv_label_create(net_ui);
    lv_label_set_text(label, "Choose your side:");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20);  

    
    // 添加服务器按钮
    lv_obj_t *server_btn = lv_btn_create(net_ui);
    lv_obj_set_size(server_btn, 150, 50);
    lv_obj_align(server_btn, LV_ALIGN_CENTER, 0, -50);
    lv_obj_t *server_label = lv_label_create(server_btn);
    lv_label_set_text(server_label, "RED");
    lv_obj_center(server_label);
    lv_obj_add_event_cb(server_btn, server_btn_callback, LV_EVENT_CLICKED, config);
    
    // 添加客户端按钮
    lv_obj_t *client_btn = lv_btn_create(net_ui);
    lv_obj_set_size(client_btn, 150, 50);
    lv_obj_align(client_btn, LV_ALIGN_CENTER, 0, 50);
    lv_obj_t *client_label = lv_label_create(client_btn);
    lv_label_set_text(client_label, "BLACK");
    lv_obj_center(client_label);
    lv_obj_add_event_cb(client_btn, client_btn_callback, LV_EVENT_CLICKED, config);
}

// 服务器按钮回调
void server_btn_callback(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    NetworkConfig *config = lv_event_get_user_data(e);
    if(code == LV_EVENT_CLICKED) {
        // 初始化网络为服务器模式
        if (network_init(&network, NETWORK_MODE_SERVER, config->ip,config->port) == 0) {
            player = RED; // 服务器执红先手
            network.room_id = -1; // 确保房间ID初始为-1
            
            printf("Initializing as RED player...\n");
            
            // 首先隐藏主界面
            lv_obj_add_flag(main_ui, LV_OBJ_FLAG_HIDDEN);
            
            // 删除网络选择界面
            if (net_ui) {
                lv_obj_del(net_ui);
                net_ui = NULL;
            }
            
            // 创建并显示等待界面
            wait_screen();
            
            // 启动连接检查计时器
            start_connection_check_timer();
            
            // 强制更新UI
            lv_timer_handler();
            
            printf("Waiting for opponent...\n");
            // // 初始化棋盘和界面
            // lv_obj_clean(wait_ui);
            // lv_obj_del(wait_ui);
            // wait_ui = NULL;
            // net_ui = NULL;

            // qizi_init();
            // game_screen();
            // lv_obj_remove_flag(game_ui, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

// 客户端按钮回调
void client_btn_callback(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    NetworkConfig *config = lv_event_get_user_data(e);
    if(code == LV_EVENT_CLICKED) {
        
        // 初始化网络为客户端模式
        if (network_init(&network, NETWORK_MODE_CLIENT, config->ip,config->port) == 0) {
            player = RED; // 客户端执黑后手
            network.room_id = -1; // 确保房间ID初始为-1
            
            printf("Initializing as BLACK player...\n");
            
            // 首先隐藏主界面
            lv_obj_add_flag(main_ui, LV_OBJ_FLAG_HIDDEN);
            
            // 删除网络选择界面
            if (net_ui) {
                lv_obj_del(net_ui);
                net_ui = NULL;
            }
            
            // 创建并显示等待界面
            wait_screen();
            
            // 启动连接检查计时器
            start_connection_check_timer();
            
            // 强制更新UI
            lv_timer_handler();
            
            printf("Waiting for opponent...\n");
            // // 初始化棋盘和界面
            // lv_obj_clean(wait_ui);
            // lv_obj_del(wait_ui);
            // wait_ui = NULL;
            // net_ui = NULL;

            // qizi_init();
            // game_screen();
            // lv_obj_remove_flag(game_ui, LV_OBJ_FLAG_HIDDEN);
        }
    }
}



/*
    * 处理接收到的网络消息
    *msg： 接收到的网络消息
*/
void process_network_message(const NetworkMessage* msg) {
    switch (msg->type) {
        case MSG_MOVE: {
            // 处理移动消息
            int from_x = msg->data[0];
            int from_y = msg->data[1];
            int to_x = msg->data[2];
            int to_y = msg->data[3];
            
            Qizi* qizi = chessboard[from_x][from_y];
            if (qizi && qizi->live) {
                // 执行移动
                move_qizi(qizi, to_x, to_y);
            }
            break;
        }
        
        case MSG_UNDO_REQUEST:
            // 处理悔棋请求
            show_undo_request_dialog();
            break;
            
        case MSG_UNDO_RESPONSE:
            // 处理悔棋响应
            handle_undo_response(msg->data[0]);
            break;
            
        case MSG_RESTART:
            // 重新开始游戏
            qizi_init();
            lv_obj_clean(game_ui);
            game_screen();
            break;
            
        case MSG_QUIT:
            // 退出游戏
            printf("对方已退出游戏\n");
            network.connected = 0;
            break;
            
        case MSG_CHAT:
            // 显示聊天消息
            //show_chat_message(msg->text);
            break;
        case MSG_SUFF:
            // 对方认输
            printf("对方认输，你赢了！\n");
            lv_obj_t * game_over = lv_image_create(game_ui);
            lv_obj_align(game_over,LV_ALIGN_CENTER,0,0);
            lv_image_set_src(game_over,"A:./pic/youlost.png");  
            break;

        case MSG_ROOM_JOIN:
            // 处理加入房间消息
            network.room_id = msg->data[0];
            printf("Joined room %d\n", network.room_id);
            break;
            
        default:
            break;
    }
}

// 悔棋超时回调
static void undo_timeout_cb(lv_timer_t *timer) {
    // 标记超时状态
    undo_responded = 1;
    undo_accepted = 0;
    
    // 安全关闭对话框
    if (undo_dialog) {
        lv_msgbox_close(undo_dialog);
        undo_dialog = NULL;
    }
    
    if (wait_msg) {
        lv_msgbox_close(wait_msg);
        wait_msg = NULL;
    }

    // 只有在还有效的请求时才显示超时消息
    if (undo_requested) {
        printf("悔棋请求超时，自动拒绝\n");
        
        if (!msg_timeout) {  // 确保不重复创建
            msg_timeout = lv_msgbox_create(NULL);
            if (msg_timeout) {  // 检查创建是否成功
                lv_msgbox_add_text(msg_timeout, "Undo Timeout!");
                lv_obj_t *btn = lv_msgbox_add_footer_button(msg_timeout, "OK");
                if (btn) {
                    lv_obj_add_event_cb(btn, undo_msg_timeout_cb, LV_EVENT_CLICKED, NULL);
                }
                lv_obj_center(msg_timeout);
            }
        }
    }

    // 清理定时器
    if (undo_timeout_timer) {
        lv_timer_del(undo_timeout_timer);
        undo_timeout_timer = NULL;
    }

    // 重置状态
    undo_requested = 0;
}

// 显示悔棋请求对话框
void show_undo_request_dialog(void) {
    if (undo_dialog) {
        lv_msgbox_close(undo_dialog);
        undo_dialog = NULL;
    }
    
    //undo_dialog = lv_msgbox_create(NULL, "悔棋请求", "对方请求悔棋，是否同意？", {"同意", "拒绝"}, true);
    undo_dialog = lv_msgbox_create(NULL);
    lv_msgbox_add_text(undo_dialog, "MSG_UNDO_REQUEST");
    lv_obj_t *btn = lv_msgbox_add_footer_button(undo_dialog, "YES");
    lv_obj_add_event_cb(btn, undo_yes_callback, LV_EVENT_CLICKED, NULL);

    btn = lv_msgbox_add_footer_button(undo_dialog, "NO");
    lv_obj_add_event_cb(btn, undo_no_callback, LV_EVENT_CLICKED, NULL);
    lv_obj_center(undo_dialog);

    
    // 设置超时计时器（30秒）
    if (undo_timeout_timer) {
        lv_timer_del(undo_timeout_timer);
    }

    undo_timeout_timer = lv_timer_create(undo_timeout_cb, 10000, NULL);
    lv_timer_set_repeat_count(undo_timeout_timer, 1);
}

void undo_msg_timeout_cb(lv_event_t *e)
{
    if (msg_timeout) {
        lv_msgbox_close(msg_timeout);
        msg_timeout = NULL;
    }
}

void undo_yes_callback(lv_event_t *e)
{
    printf("%d\n",__LINE__);
    send_undo_response(1);
    if(undo_dialog)
        lv_msgbox_close(undo_dialog);
    //lv_msgbox_close(undo_dialog);
    undo_dialog = NULL;
    printf("%d\n",__LINE__);

    // 本地也执行悔棋
    printf("开始悔棋\n");
    undo_move();
    printf("%d\n",__LINE__);
    
    if (undo_timeout_timer) {
        lv_timer_del(undo_timeout_timer);
        undo_timeout_timer = NULL;
    }
    printf("%d\n",__LINE__);
}

void undo_no_callback(lv_event_t *e)
{
    send_undo_response(0);
    if(undo_dialog)
        lv_msgbox_close(undo_dialog);
    //lv_msgbox_close(undo_dialog);
    printf("%d\n",__LINE__);
    undo_dialog = NULL;

    printf("否定悔棋\n");
    
    if (undo_timeout_timer) {
        lv_timer_del(undo_timeout_timer);
        undo_timeout_timer = NULL;
    }
}

// // 悔棋对话框回调
// void undo_dialog_callback(lv_event_t *e) {
//     lv_obj_t *mbox = lv_event_get_target(e);
//     printf("%d\n",__LINE__);
//     uint16_t btn_id = lv_msgbox_get_footer(mbox);
//     printf("btn_id = %d",btn_id);
//     printf("%d\n",__LINE__);

//     if (btn_id == 0) { // 同意
//         printf("%d\n",__LINE__);
//         send_undo_response(1);
//         // 本地也执行悔棋
//         undo_move();
//         printf("%d\n",__LINE__);
//     } else { // 拒绝
//         printf("%d\n",__LINE__);;
//         send_undo_response(0);
//         printf("%d\n",__LINE__);
//     }
    
//     lv_obj_del(undo_dialog);
//     printf("%d\n",__LINE__);
//     undo_dialog = NULL;
    
//     if (undo_timeout_timer) {
//         lv_timer_del(undo_timeout_timer);
//         undo_timeout_timer = NULL;
//     }
// }

// 发送悔棋请求
void send_undo_request(void) {
    if (network.connected) {
        NetworkMessage msg;
        msg.type = MSG_UNDO_REQUEST;
        network_send(&network, &msg);
        
        // 显示等待响应提示
        //lv_obj_t *wait_msg = lv_msgbox_create(NULL, "等待响应", "已发送悔棋请求，等待对方响应...", NULL, false);
        wait_msg = lv_msgbox_create(NULL);
        lv_msgbox_add_text(wait_msg, "Waiting for opponent's response...");

        lv_obj_center(wait_msg);
        
        // 设置超时计时器
        if (undo_timeout_timer) {
            lv_timer_del(undo_timeout_timer);
        }
        undo_timeout_timer = lv_timer_create(undo_timeout_cb, 10000, NULL);
        lv_timer_set_repeat_count(undo_timeout_timer, 1);
        
        undo_requested = 1;
        undo_responded = 0;
    }
}

// 发送悔棋响应
void send_undo_response(int accepted) {
    if (network.connected) {
        NetworkMessage msg;
        msg.type = MSG_UNDO_RESPONSE;
        msg.data[0] = accepted;
        network_send(&network, &msg);
        
        undo_responded = 1;
        undo_accepted = accepted;
    }
}

// 关闭等待消息回调
void undo_msg_acc_cb(lv_event_t *e) {
    if (msg_acc) {
        lv_msgbox_close(msg_acc);
        msg_acc = NULL;
    }

    if (wait_msg) {
        lv_msgbox_close(wait_msg);
        wait_msg = NULL;
    }
    
    if (undo_timeout_timer) {
        lv_timer_del(undo_timeout_timer);
        undo_timeout_timer = NULL;
    }
}

void undo_msg_rej_cb(lv_event_t *e) {
    if (msg_rej) {
        lv_msgbox_close(msg_rej);
        msg_rej = NULL;
    }

    if (wait_msg) {
        lv_msgbox_close(wait_msg);
        wait_msg = NULL;
    }
    
    if (undo_timeout_timer) {
        lv_timer_del(undo_timeout_timer);
        undo_timeout_timer = NULL;
    }
}

// 处理悔棋响应
void handle_undo_response(int accepted) {
    if (undo_timeout_timer) {
        lv_timer_del(undo_timeout_timer);
        undo_timeout_timer = NULL;
    }
    
    if (accepted) {
        // 对方同意悔棋，执行悔棋操作
        printf("对方同意了您的悔棋请求\n");
        undo_move();
        
        // 显示同意提示
        //lv_obj_t *msg = lv_msgbox_create(NULL, "悔棋已同意", "对方同意了您的悔棋请求", {"确定"}, true);
        msg_acc = lv_msgbox_create(NULL);
        lv_msgbox_add_text(msg_acc, "MSG_UNDO_ACCEPTED");
        lv_obj_t *btn = lv_msgbox_add_footer_button(msg_acc, "OK");

        lv_obj_center(msg_acc);

        lv_obj_add_event_cb(btn, undo_msg_acc_cb, LV_EVENT_CLICKED, NULL);
    } else {
        // 对方拒绝悔棋
        //lv_obj_t *msg = lv_msgbox_create(NULL, "悔棋被拒绝", "对方拒绝了您的悔棋请求", {"确定"}, true);
        printf("对方拒绝了您的悔棋请求\n");
        msg_rej = lv_msgbox_create(NULL);
        lv_msgbox_add_text(msg_rej, "MSG_UNDO_DECLINED");
        lv_obj_t *btn = lv_msgbox_add_footer_button(msg_rej, "OK");
        lv_obj_center(msg_rej);
        lv_obj_add_event_cb(btn, undo_msg_rej_cb, LV_EVENT_CLICKED, NULL);
    }
    
    undo_requested = 0;
    undo_responded = 1;
    undo_accepted = accepted;
}

// 网络悔棋函数
void network_undo_move(void) {
    if (network.connected) {
        // 网络模式下发送悔棋请求

        if(undo_requested) {
            // 已经有未处理的悔棋请求，忽略新的请求
            return;
        }
        send_undo_request();
    } else {
        // 单机模式下直接执行悔棋
        undo_move();
    }
}


