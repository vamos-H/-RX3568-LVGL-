#include "xiangqi.h"
#include "chess.h"
#include <stdio.h>
#include <stdlib.h>

int player = RED; // 当前玩家，RED或BLACK

Qizi* chessboard[10][9]; // 棋盘二维数组

Qizi* selected_qizi = NULL; // 全局变量，指向当前被选中的棋子



Qizi red_s = {NULL,NULL,"A:./pic/red_w.png",9,4,1,RED,QZ_J,false};

Qizi red_s1 = {NULL,NULL,"A:./pic/red_s.png",9,3,1,RED,QZ_S,false};

Qizi red_s2 = {NULL,NULL,"A:./pic/red_s.png",9,5,1,RED,QZ_S,false};

Qizi red_x1 = {NULL,NULL,"A:./pic/red_x.png",9,2,1,RED,QZ_X,false};

Qizi red_x2 = {NULL,NULL,"A:./pic/red_x.png",9,6,1,RED,QZ_X,false};

Qizi red_m1 = {NULL,NULL,"A:./pic/red_m.png",9,1,1,RED,QZ_M,false};

Qizi red_m2 = {NULL,NULL,"A:./pic/red_m.png",9,7,1,RED,QZ_M,false};

Qizi red_c1 = {NULL,NULL,"A:./pic/red_c.png",9,0,1,RED,QZ_C,false};

Qizi red_c2 = {NULL,NULL,"A:./pic/red_c.png",9,8,1,RED,QZ_C,false};

Qizi red_p1 = {NULL,NULL,"A:./pic/red_p.png",7,1,1,RED,QZ_P,false};     //

Qizi red_p2 = {NULL,NULL,"A:./pic/red_p.png",7,7,1,RED,QZ_P,false};

Qizi red_b1 = {NULL,NULL,"A:./pic/red_b.png",6,0,1,RED,QZ_B,false};

Qizi red_b2 = {NULL,NULL,"A:./pic/red_b.png",6,2,1,RED,QZ_B,false};

Qizi red_b3 = {NULL,NULL,"A:./pic/red_b.png",6,4,1,RED,QZ_B,false};

Qizi red_b4 = {NULL,NULL,"A:./pic/red_b.png",6,6,1,RED,QZ_B,false};

Qizi red_b5 = {NULL,NULL,"A:./pic/red_b.png",6,8,1,RED,QZ_B,false};

Qizi  black_j = {NULL,NULL,"A:./pic/black_j.png",0,4,1,BLACK,QZ_J,false};

Qizi  black_s1 = {NULL,NULL,"A:./pic/black_s.png",0,3,1,BLACK,QZ_S,false};

Qizi  black_s2 = {NULL,NULL,"A:./pic/black_s.png",0,5,1,BLACK,QZ_S,false};

Qizi  black_x1 = {NULL,NULL,"A:./pic/black_x.png",0,2,1,BLACK,QZ_X,false};

Qizi  black_x2 = {NULL,NULL,"A:./pic/black_x.png",0,6,1,BLACK,QZ_X,false};

Qizi  black_m1 = {NULL,NULL,"A:./pic/black_m.png",0,1,1,BLACK,QZ_M,false};

Qizi  black_m2 = {NULL,NULL,"A:./pic/black_m.png",0,7,1,BLACK,QZ_M,false};

Qizi  black_c1 = {NULL,NULL,"A:./pic/black_c.png",0,0,1,BLACK,QZ_C,false};

Qizi  black_c2 = {NULL,NULL,"A:./pic/black_c.png",0,8,1,BLACK,QZ_C,false};

Qizi  black_p1 = {NULL,NULL,"A:./pic/black_p.png",2,1,1,BLACK,QZ_P,false};

Qizi  black_p2 = {NULL,NULL,"A:./pic/black_p.png",2,7,1,BLACK,QZ_P,false};

Qizi  black_b1 = {NULL,NULL,"A:./pic/black_b.png",3,0,1,BLACK,QZ_B,false};

Qizi  black_b2 = {NULL,NULL,"A:./pic/black_b.png",3,2,1,BLACK,QZ_B,false};

Qizi  black_b3 = {NULL,NULL,"A:./pic/black_b.png",3,4,1,BLACK,QZ_B,false};

Qizi  black_b4 = {NULL,NULL,"A:./pic/black_b.png",3,6,1,BLACK,QZ_B,false};

Qizi  black_b5 = {NULL,NULL,"A:./pic/black_b.png",3,8,1,BLACK,QZ_B,false};



void insert_huiqi(void)
{
    

}



void qizi_init(void)
{
    move_count = 0;
    current_move = 0;
    for(int r = 0; r < 10; r++)
    {
        for(int c = 0; c < 9; c++)
        {
            chessboard[r][c] = NULL;
        }
    }

    red_s.x = 9;
    red_s.y = 4;
    red_s.live = 1;

    red_s1.x = 9;
    red_s1.y = 3;
    red_s1.live = 1;

    red_s2.x = 9;
    red_s2.y = 5;
    red_s2.live = 1;

    red_x1.x = 9;
    red_x1.y = 2;
    red_x1.live = 1;

    red_x2.x = 9;
    red_x2.y = 6;
    red_x2.live = 1;

    red_m1.x = 9;
    red_m1.y = 1;
    red_m1.live = 1;

    red_m2.x = 9;
    red_m2.y = 7;
    red_m2.live = 1;

    red_c1.x = 9;
    red_c1.y = 0;
    red_c1.live = 1;

    red_c2.x = 9;
    red_c2.y = 8;
    red_c2.live = 1;

    red_p1.x = 7;
    red_p1.y = 1;
    red_p1.live = 1;

    red_p2.x = 7;
    red_p2.y = 7;
    red_p2.live = 1;

    red_b1.x = 6;
    red_b1.y = 0;
    red_b1.live = 1;

    red_b2.x = 6;
    red_b2.y = 2;
    red_b2.live = 1;

    red_b3.x = 6;
    red_b3.y = 4;
    red_b3.live = 1;

    red_b4.x = 6;
    red_b4.y = 6;
    red_b4.live = 1;
    
    red_b5.x = 6;
    red_b5.y = 8;
    red_b5.live = 1;

    black_j.x = 0;
    black_j.y = 4;
    black_j.live = 1;

    black_s1.x = 0;
    black_s1.y = 3;
    black_s1.live = 1;

    black_s2.x = 0;
    black_s2.y = 5;
    black_s2.live = 1;

    black_x1.x = 0;
    black_x1.y = 2;
    black_x1.live = 1;

    black_x2.x = 0;
    black_x2.y = 6;
    black_x2.live = 1;

    black_m1.x = 0;
    black_m1.y = 1;
    black_m1.live = 1;

    black_m2.x = 0;
    black_m2.y = 7;
    black_m2.live = 1;

    black_c1.x = 0;
    black_c1.y = 0;
    black_c1.live = 1;

    black_c2.x = 0;
    black_c2.y = 8;
    black_c2.live = 1;

    black_p1.x = 2;
    black_p1.y = 1;
    black_p1.live = 1;

    black_p2.x = 2;
    black_p2.y = 7;
    black_p2.live = 1;

    black_b1.x = 3;
    black_b1.y = 0;
    black_b1.live = 1;

    black_b2.x = 3;
    black_b2.y = 2;
    black_b2.live = 1;

    black_b3.x = 3;
    black_b3.y = 4;
    black_b3.live = 1;

    black_b4.x = 3;
    black_b4.y = 6;
    black_b4.live = 1;
    
    black_b5.x = 3;
    black_b5.y = 8;
    black_b5.live = 1;

    // 将所有棋子放到棋盘上的对应位置
    chessboard[red_s.x][red_s.y] = &red_s;
    chessboard[red_s1.x][red_s1.y] = &red_s1;
    chessboard[red_s2.x][red_s2.y] = &red_s2;
    chessboard[red_x1.x][red_x1.y] = &red_x1;
    chessboard[red_x2.x][red_x2.y] = &red_x2;
    chessboard[red_m1.x][red_m1.y] = &red_m1;
    chessboard[red_m2.x][red_m2.y] = &red_m2;
    chessboard[red_c1.x][red_c1.y] = &red_c1;
    chessboard[red_c2.x][red_c2.y] = &red_c2;
    chessboard[red_p1.x][red_p1.y] = &red_p1;
    chessboard[red_p2.x][red_p2.y] = &red_p2;
    chessboard[red_b1.x][red_b1.y] = &red_b1;
    chessboard[red_b2.x][red_b2.y] = &red_b2;
    chessboard[red_b3.x][red_b3.y] = &red_b3;
    chessboard[red_b4.x][red_b4.y] = &red_b4;
    chessboard[red_b5.x][red_b5.y] = &red_b5;

    chessboard[black_j.x][black_j.y] = &black_j;
    chessboard[black_s1.x][black_s1.y] = &black_s1;
    chessboard[black_s2.x][black_s2.y] = &black_s2;
    chessboard[black_x1.x][black_x1.y] = &black_x1;
    chessboard[black_x2.x][black_x2.y] = &black_x2;
    chessboard[black_m1.x][black_m1.y] = &black_m1;
    chessboard[black_m2.x][black_m2.y] = &black_m2;
    chessboard[black_c1.x][black_c1.y] = &black_c1;
    chessboard[black_c2.x][black_c2.y] = &black_c2;
    chessboard[black_p1.x][black_p1.y] = &black_p1;
    chessboard[black_p2.x][black_p2.y] = &black_p2;
    chessboard[black_b1.x][black_b1.y] = &black_b1;
    chessboard[black_b2.x][black_b2.y] = &black_b2;
    chessboard[black_b3.x][black_b3.y] = &black_b3;
    chessboard[black_b4.x][black_b4.y] = &black_b4;
    chessboard[black_b5.x][black_b5.y] = &black_b5;

}