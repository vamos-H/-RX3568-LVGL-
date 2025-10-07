#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "network.h"
#include "chess.h"
#include <errno.h>

extern NetworkState network;

void* network_recv_thread(void* arg) {
    NetworkState* net = (NetworkState*)arg;
    NetworkMessage msg;
    
    while (net->connected) {
        if (network_receive(net, &msg) > 0) {
            // 处理接收到的消息
            printf("\n");
            printf("recv msg type=%d\n", msg.type);
            printf("recv msg data=%d %d %d %d\n", msg.data[0], msg.data[1], msg.data[2], msg.data[3]);
            printf("\n");
            process_network_message(&msg);
        } else {
            usleep(10000); // 10ms
        }
    }
    
    return NULL;
}
/*
    network_init：初始化网络连接
    mode: NETWORK_MODE_SERVER 或 NETWORK_MODE_CLIENT
    ip: 服务器IP地址（客户端模式下使用）
    port: 服务器端口号
*/
int network_init(NetworkState* net, NetworkMode mode, const char* ip,const char* port) 
{
    printf("%s %s %d\n",ip,port,mode);
    memset(net, 0, sizeof(NetworkState));
    network.room_id = -1; // 初始化房间ID为-1
    net->mode = mode;
    net->connected = 0;
    
    if (mode == NETWORK_MODE_SERVER) {
        // RED服务器模式
        struct sockaddr_in serv_addr;
        
        if ((net->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("\n Socket creation error \n");
            return -1;
        }
        
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(atoi(port));
        serv_addr.sin_addr.s_addr = inet_addr(ip);

        // 连接服务器
        if (connect(net->sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            printf("\nConnection Failed \n");
            return -1;
        }
        
        net->connected = 1;

        printf("已连接到服务器\n");
        NetworkMessage join_msg;
        join_msg.type = MSG_ROOM_JOIN;
        join_msg.data[0] = RED; // RED服务器
        network_send(net, &join_msg);
        
    } else if (mode == NETWORK_MODE_CLIENT) {
        // 客户端模式
        struct sockaddr_in serv_addr;
        
        if ((net->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("\n Socket creation error \n");
            return -1;
        }
        
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(atoi(port));
        serv_addr.sin_addr.s_addr = inet_addr(ip);

        // 连接服务器
        if (connect(net->sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            printf("\nConnection Failed \n");
            return -1;
        }
        
        net->connected = 1;
        printf("已连接到服务器\n");

        NetworkMessage join_msg;
        join_msg.type = MSG_ROOM_JOIN;
        join_msg.data[0] = BLACK; // BLACK客户端
        network_send(net, &join_msg);
    }
    
    // 创建接收线程
    if (pthread_create(&net->recv_thread, NULL, network_recv_thread, net) != 0) {
        printf("Failed to create receive thread\n");
        return -1;
    }
    
    return 0;
}


void network_cleanup(NetworkState* net)
{
    net->connected = 0;
    if (net->recv_thread) {
        pthread_join(net->recv_thread, NULL);
    }
    close(net->sockfd);
    printf("网络已断开\n");
}

int network_send(NetworkState* net, const NetworkMessage* msg) {
    if (!net->connected) return -1;
    
    return send(net->sockfd, msg, sizeof(NetworkMessage), 0);
}

int network_receive(NetworkState* net, NetworkMessage* msg) {
    if (!net->connected) return -1;
    
    return recv(net->sockfd, msg, sizeof(NetworkMessage), 0);
}