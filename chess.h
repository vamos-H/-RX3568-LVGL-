#ifndef CHESS_H_
#define CHESS_H_

#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include "xiangqi.h"

extern lv_obj_t *main_ui ;
extern lv_obj_t *game_ui ;
extern lv_obj_t *over_ui ;

// void display_chi1_image(void) ;

// void display_chi2_image(void) ;

static void hide_chi_image_cb(lv_timer_t *timer) ;

// void show_chi_effect(void) ;

void display_chi_image(int captured_camp) ;

void show_chi_effect(int captured_camp) ;

void init_screen_click() ;

void button_s_callback(lv_event_t *e);

void main_screen(void);

void button_q_callback(lv_event_t *e);

void game_screen(void);

void qizi_event_handler(lv_event_t *e);

int movement_che(Qizi *qizi,int target_x,int target_y);

int movement_ma(Qizi *qizi,int target_x,int target_y);

int movement_xiang(Qizi *qizi,int target_x,int target_y);

int movement_shi(Qizi *qizi,int target_x,int target_y);

int movement_jiang(Qizi *qizi,int target_x,int target_y);

int movement_bing(Qizi *qizi,int target_x,int target_y);

int movement_pao(Qizi *qizi,int target_x,int target_y);

void move_qizi(Qizi* qizi, int target_x, int target_y);

Qizi* get_qizi_by_lv_obj(lv_obj_t* obj) ;


static void anim_completed_cb(lv_anim_t * anim) ;

static void set_qizi_pos_anim(void * var, int32_t value_x, int32_t value_y) ;

void animate_qizi_move(Qizi* qizi, int target_x, int target_y) ;

// 移动历史记录结构
typedef struct {
    Qizi* piece;        // 移动的棋子
    int from_x;         // 起始位置x
    int from_y;         // 起始位置y
    int to_x;           // 目标位置x
    int to_y;           // 目标位置y
    Qizi* captured;     // 被吃的棋子（如果有）
    int captured_live;  // 被吃棋子的存活状态
} MoveHistory;

// 添加这些全局变量声明
extern MoveHistory move_history[500];  // 移动历史记录
extern int move_count;                 // 当前移动步数
extern int current_move;               // 当前显示的移动步数

// 添加悔棋功能函数声明
void record_move(Qizi* piece, int from_x, int from_y, int to_x, int to_y, Qizi* captured);
void undo_move(void);
void redo_move(void);
void update_undo_redo_buttons(void);

static void undo_btn_event_cb(lv_event_t * e) ;
static void redo_btn_event_cb(lv_event_t * e) ;

#endif