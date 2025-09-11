#ifndef NETWORK_H_
#define NETWORK_H_

#define MAX_BUFFER_SIZE 256

typedef enum {
    NETWORK_MODE_NONE = 0,  //单机模式
    NETWORK_MODE_SERVER,    // 服务器模式
    NETWORK_MODE_CLIENT     // 客户端模式
} NetworkMode;              // 网络模式

typedef struct {
    int sockfd;             // 套接字文件描述符
    NetworkMode mode;
    int connected;          // 连接状态
    pthread_t recv_thread;  // 接收线程
} NetworkState;             // 网络状态


typedef enum {
    MSG_MOVE = 1,       // 移动棋子
    MSG_UNDO_REQUEST,   // 悔棋请求
    MSG_UNDO_RESPONSE,  // 悔棋响应
    MSG_RESTART,        // 重新开始
    MSG_QUIT,           // 退出游戏
    MSG_CHAT            // 聊天消息
} MessageType;          // 网络消息类型

// 网络消息结构
typedef struct {
    MessageType type;
    int data[4];        // 用于存储坐标等数据
    char text[64];      // 用于存储文本消息
} NetworkMessage;       // 网络消息结构

// 函数声明
int network_init(NetworkState* net, NetworkMode mode, const char* ip);
void network_cleanup(NetworkState* net);
int network_send(NetworkState* net, const NetworkMessage* msg);
int network_receive(NetworkState* net, NetworkMessage* msg);
void* network_recv_thread(void* arg);

#endif