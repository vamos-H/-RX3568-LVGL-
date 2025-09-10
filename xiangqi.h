#ifndef XIANGQI_H_
#define XIANGQI_H_

#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"

#define RED 0
#define BLACK 1

typedef struct qizi
{
    lv_obj_t *scr;
    lv_obj_t *pic;
    char pic_add[50];   //棋子图片所在的地址
    int x;              //棋子在棋盘上对应的x轴
    int y;              //棋子在棋盘上对应的y轴
    int live;           //棋子是否存活
    int camp;           //棋子所属阵营
    int type;           //棋子类型
    bool selected;      //是否被选择
}Qizi;

typedef enum {
    QZ_NONE = 0, // 空格
    QZ_J,    // 将/帅
    QZ_S,      // 士/仕
    QZ_X,    // 象/相
    QZ_C,      // 车
    QZ_M,       // 马
    QZ_P,      // 炮
    QZ_B,     // 兵/卒
} QiziType;


typedef struct seqstack
{
    Qizi *chess_add[10][9];
    int top;
    int max_len;
}Seqstack;

extern int player;

extern Qizi* selected_qizi;

extern Qizi* chessboard[10][9];

extern Qizi red_s;
extern Qizi red_s1;
extern Qizi red_s2;
extern Qizi red_x1;
extern Qizi red_x2;
extern Qizi red_m1;
extern Qizi red_m2;
extern Qizi red_c1;
extern Qizi red_c2;
extern Qizi red_p1;
extern Qizi red_p2;
extern Qizi red_b1;
extern Qizi red_b2;
extern Qizi red_b3;
extern Qizi red_b4;
extern Qizi red_b5;

extern Qizi black_j;
extern Qizi black_s1;
extern Qizi black_s2;
extern Qizi black_x1;
extern Qizi black_x2;
extern Qizi black_m1;
extern Qizi black_m2;
extern Qizi black_c1;
extern Qizi black_c2;
extern Qizi black_p1;
extern Qizi black_p2;
extern Qizi black_b1;
extern Qizi black_b2;
extern Qizi black_b3;
extern Qizi black_b4;
extern Qizi black_b5;

void qizi_init(void);

#endif