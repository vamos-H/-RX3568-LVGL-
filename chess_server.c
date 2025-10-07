/*
    游戏服务器代码 独立与其余文件
*/
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <sys/socket.h>
#include <linux/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define RED 0
#define BLACK 1



typedef enum {
    MSG_MOVE = 1,       // 移动棋子
    MSG_UNDO_REQUEST,   // 悔棋请求
    MSG_UNDO_RESPONSE,  // 悔棋响应
    MSG_RESTART,        // 重新开始
    MSG_QUIT,           // 退出游戏
    MSG_CHAT,           // 聊天消息
    MSG_ROOM_JOIN,      // 加入房间
    MSG_ROOM_LEAVE,     // 离开房间
    MSG_SUFF            // 认输
} MessageType;          // 网络消息类型

// 网络消息结构
typedef struct {
    MessageType type;
    int data[4];        // 用于存储坐标等数据
    char text[64];      // 用于存储文本消息
} NetworkMessage;       // 网络消息结构


int sockfd; //全局套接字

int sockfd_red[50];
int sockfd_black[50];
int room[50];
int red_count=0;
int black_count=0;
int room_count=0;
int sockfd_count=0;
int current_room=0;

pthread_t thread_room[50];


pthread_mutex_t mutex;

void server_init(const char *ip, const char *port)
{
    int r;
    /*step1: socket 创建一个流式套接字接口*/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    /*step2: bind*/
    struct sockaddr_in my_addr;//定义一个以太网地址结构体
    memset(&my_addr, 0, sizeof(my_addr));//把结构体清空
    my_addr.sin_family = AF_INET;//IPV4协议
    my_addr.sin_port = htons(atoi(port));
    my_addr.sin_addr.s_addr = inet_addr(ip);
    r = bind(sockfd, (struct sockaddr*)&my_addr, sizeof(my_addr));
    if(r != 0)
    {
        perror("bind error");
        return;
    }
    /*step3:listen*/
    r = listen(sockfd, 100);
    if(r != 0)
    {
        perror("listen error");
        return;
    }

    memset(sockfd_red,-1,sizeof(sockfd_red));
    memset(sockfd_black,-1,sizeof(sockfd_black));
    memset(room,-1,sizeof(room));
}


void server_uinit(void)
{
    close(sockfd);
}

void handle_unconnect(int room_id)
{
    int red_sockfd = sockfd_red[room_id];
    int black_sockfd = sockfd_black[room_id];

    close(red_sockfd);
    close(black_sockfd);
    room[room_id] = -1;
    sockfd_red[room_id] = -1;
    sockfd_black[room_id] = -1;
    red_count--;
    black_count--;
    sockfd_count -= 2;
    room_count--;
}

void *black_send_handler(void *arg)
{
    pthread_detach(pthread_self());//设置分离属性
    int room_id = *(int*)arg;
    int black_sockfd = sockfd_black[room_id];
    int red_sockfd = sockfd_red[room_id];

    while(1)
    {
        if(room[room_id]==-1)
            break;
            
        NetworkMessage msg;
        memset(&msg, 0, sizeof(msg));
        printf("room %d waiting black player msg\n", room_id);
        
        // 使用select来进行非阻塞读取
        fd_set readfds;
        struct timeval tv;
        FD_ZERO(&readfds);
        FD_SET(black_sockfd, &readfds);
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        
        int ret = select(black_sockfd + 1, &readfds, NULL, NULL, &tv);
        if(ret > 0 && FD_ISSET(black_sockfd, &readfds))
        {
            ssize_t r = recv(black_sockfd, &msg, sizeof(msg), MSG_NOSIGNAL);
            printf("Black player received %zd bytes\n", r);
            
            if(r > 0)
            {
                printf("room %d recv black msg type=%d\n", room_id, msg.type);
                printf("room %d recv black msg data=%d %d %d %d\n", room_id, msg.data[0], msg.data[1], msg.data[2], msg.data[3]);
                // 转发给红方
                write(red_sockfd, &msg, sizeof(msg));
                
                // 如果是退出或认输消息，关闭房间
                if(msg.type == MSG_QUIT || msg.type == MSG_SUFF) {
                    printf("Game ended in room %d\n", room_id);
                    handle_unconnect(room_id);
                    break;
                }
            }
            else if(r == 0)
            {
                printf("room %d black player disconnected\n", room_id);
                handle_unconnect(room_id);
                break;
            }
        }
    }
}

void *room_handler(void *arg)
{
    pthread_detach(pthread_self());//设置分离属性
    
    // 使用传入的房间ID
    int *room_id_ptr = (int*)arg;
    int room_id = *room_id_ptr;
    // 不再需要时释放内存
    free(room_id_ptr);

    int red_sockfd = sockfd_red[room_id];
    int black_sockfd = sockfd_black[room_id];
    printf("room handler started for room %d, red_sock=%d, black_sock=%d\n", 
           room_id, red_sockfd, black_sockfd);


    NetworkMessage start_msg;
    memset(&start_msg,0,sizeof(NetworkMessage));
    start_msg.type = MSG_ROOM_JOIN;
    write(red_sockfd, &start_msg, sizeof(start_msg));
    write(black_sockfd, &start_msg, sizeof(start_msg));

    // 为black_handler创建新的房间ID副本
    int *black_room_id = malloc(sizeof(int));
    if(black_room_id) {
        *black_room_id = room_id;
        pthread_t thread;
        pthread_create(&thread, NULL, black_send_handler, black_room_id);
    }

    while(1)
    {
        if(room[room_id] == -1)
            break;
            
        NetworkMessage msg;
        memset(&msg, 0, sizeof(msg));
        printf("room %d waiting red player msg\n", room_id);
        
        // 使用select来进行非阻塞读取
        fd_set readfds;
        struct timeval tv;
        FD_ZERO(&readfds);
        FD_SET(red_sockfd, &readfds);
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        
        int ret = select(red_sockfd + 1, &readfds, NULL, NULL, &tv);
        if(ret > 0 && FD_ISSET(red_sockfd, &readfds))
        {
            ssize_t r = recv(red_sockfd, &msg, sizeof(msg), MSG_NOSIGNAL);
            printf("Red player received %zd bytes\n", r);
            if(r > 0)
            {
                printf("room %d recv red msg type=%d\n", room_id, msg.type);
                printf("room %d recv red msg data=%d %d %d %d\n", room_id, msg.data[0], msg.data[1], msg.data[2], msg.data[3]);
                
                // 转发给黑方
                write(black_sockfd, &msg, sizeof(msg));
                
                // 如果是退出或认输消息，关闭房间
                if(msg.type == MSG_QUIT || msg.type == MSG_SUFF) {
                    printf("Game ended in room %d\n", room_id);
                    handle_unconnect(room_id);
                    break;
                }
            }else if(r == 0)
            {
                printf("room %d red player disconnected\n", room_id);
                handle_unconnect(room_id);
                break;
            }
        }
    }
}


void *connect_hanlder(void *arg)
{
    pthread_detach(pthread_self());//设置分离属性
    while(1)
    {
        pthread_mutex_lock(&mutex);
        for(int i = 0; i < 50; i++)
        {
            if(room[i]==1)
                continue;
            if(sockfd_red[i]>0 && sockfd_black[i]>0)
            {
                room[i]=1;
                pthread_create(&thread_room[i],NULL,room_handler,(void*)&i);
                printf("create room %d\n",i);
                room_count++;
                current_room=i;
                break;
            }
        }
        sleep(1);
    }
}


int main(int argc,const char **argv)
{

    if(argc<3)
    {
        perror("请输入服务器ip和端口号\n");
        return -1;
    }

    pthread_t thread;
    int ac_sockfd;
    server_init(argv[1],argv[2]);
    
    // 初始化互斥锁
    pthread_mutex_init(&mutex, NULL);

    //pthread_create(&thread,NULL,connect_hanlder,NULL);

    while(1)
    {
        struct sockaddr_in client_addr;//保存客户端IP地址
        memset(&client_addr, 0, sizeof(client_addr));
        socklen_t addrlen = sizeof(client_addr);
        
        // 接受新的连接
        ac_sockfd = accept(sockfd, (struct sockaddr*)&client_addr, &addrlen);
        if(ac_sockfd < 0)
        {
            perror("accept error");
            continue;
        }
        printf("connected from %s[%d]\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        pthread_mutex_lock(&mutex);
        // 保存新的socket
        if(sockfd_count < 100)
        {
            NetworkMessage msg;
            int r = read(ac_sockfd, &msg, sizeof(msg));
            if(r > 0 && msg.type == MSG_ROOM_JOIN)
            {
                printf("recv msg type=%d\n", msg.type);
                printf("recv msg data=%d %d %d %d\n", msg.data[0], msg.data[1], msg.data[2], msg.data[3]);
                if(msg.data[0] == RED)
                {
                    sockfd_red[current_room] = ac_sockfd;
                    red_count++;
                }
                else if(msg.data[0] == BLACK)
                {
                    sockfd_black[current_room] = ac_sockfd;
                    black_count++;
                }
                sockfd_count++;
                
                // 当一个房间凑齐两个玩家时
                if(sockfd_red[current_room] != -1 && sockfd_black[current_room] != -1)
                {
                    room[current_room] = 1;
                    room_count++;
                    // 创建一个房间ID的副本
                    int *room_id = malloc(sizeof(int));
                    if(room_id) {
                        *room_id = current_room;
                        // 创建房间处理线程
                        pthread_create(&thread_room[current_room], NULL, room_handler, (void*)room_id);
                        current_room++;
                    }
                }
            }
        }
        pthread_mutex_unlock(&mutex);
    }

    server_uinit();

}
