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
    net->mode = mode;
    net->connected = 0;
    
    if (mode == NETWORK_MODE_SERVER) {
        // 服务器模式
        int server_fd, new_socket;
        struct sockaddr_in address;
        
        // 创建socket
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("socket failed");
            return -1;
        }
        
        memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = inet_addr(ip);
        address.sin_port = htons(atoi(port));
        int addrlen = sizeof(address);
        
        // 绑定
        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
            perror("bind failed1");
            printf("errno=%d\n",errno);
            return -1;
        }
        
        // 监听
        if (listen(server_fd, 1) < 0) {
            perror("listen");
            return -1;
        }
        
        printf("等待客户端连接...\n");
        
        // 接受连接
        struct sockaddr_in client_addr;//保存客户端IP地址
        memset(&client_addr, 0, sizeof(client_addr));
        socklen_t client_addrlen = sizeof(client_addr);
        if ((new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_addrlen)) < 0) {
            perror("accept");
            return -1;
        }
        
        net->sockfd = new_socket;
        net->connected = 1;
        
        printf("客户端已连接\n");
        
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
    }
    
    // 创建接收线程
    if (pthread_create(&net->recv_thread, NULL, network_recv_thread, net) != 0) {
        printf("Failed to create receive thread\n");
        return -1;
    }
    
    return 0;
}

void network_cleanup(NetworkState* net) {
    net->connected = 0;
    if (net->recv_thread) {
        pthread_join(net->recv_thread, NULL);
    }
    close(net->sockfd);
}

int network_send(NetworkState* net, const NetworkMessage* msg) {
    if (!net->connected) return -1;
    
    return send(net->sockfd, msg, sizeof(NetworkMessage), 0);
}

int network_receive(NetworkState* net, NetworkMessage* msg) {
    if (!net->connected) return -1;
    
    return recv(net->sockfd, msg, sizeof(NetworkMessage), 0);
}