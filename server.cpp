/*************************************************************************
	> File Name: server.cpp
	> Author: 
	> Mail: 
	> Created Time: 2017年05月13日 星期六 12时24分30秒
 ************************************************************************/

#include<iostream>
#include<cstring>
#include<cstdio>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/epoll.h>
#include<cstdlib>
#include"config.h"


int main(int argc,char **argv){

    if(argc < 2){
        std::cerr<<"need input server port!"<<std::endl;
        exit(1);
    }

    char buff[BUFF_SIZE];
    const char *ip = "127.0.0.1";
    int port = atoi(argv[1]);
    printf("server wait:ip = %s,port = %d",ip,atoi(argv[1]));

    struct sockaddr_in server_address;
    memset(&server_address,0,sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    if(inet_pton(AF_INET,ip,&server_address.sin_addr) == -1){
        std::cerr<<"inet_pton error"<<std::endl;
        exit(1);
    }

    int listenfd = socket(AF_INET,SOCK_STREAM,0);//创建
    if(listenfd == -1){
        std::cerr<<"create sockfd throw error!"<<std::endl;
        exit(1);
    }

    int ret = 0;
    ret = bind(listenfd,(struct sockaddr *)&server_address,sizeof(server_address));//绑定
    if(ret == -1){
        std::cerr<<"bind error!"<<std::endl;
        exit(1);
    }

    ret = listen(listenfd,10);//监听

    if(ret == -1){
        std::cerr<<"listen error!"<<std::endl;
        exit(1);
    }

    struct epoll_event events[MAX_EVENT_NUMBER];
    int epoll_fd = epoll_create(10);
    if(epoll_fd == -1){
        std::cerr<<"create_epoll error!"<<std::endl;
        exit(1);
    }
    addfd(epoll_fd,listenfd);

    while(1){
        int ret = epoll_wait(epoll_fd,events,MAX_EVENT_NUMBER,-1);
        if(ret < 0){
            std::cout<<"epoll_wait error!"<<std::endl;
            exit(1);
        }

        for(int i = 0;i < ret; i++){
            int sockfd = events[i].data.fd;
            if(sockfd == listenfd){
                struct sockaddr_in client_address;
                socklen_t client_address_len = sizeof(client_address);
                int connect_fd = accept(listenfd,(struct sockaddr *)&client_address,&client_address_len);
                addfd(epoll_fd,connect_fd);
            }
            else if(events[i].events & EPOLLIN){
                while(read(sockfd,buff,BUFF_SIZE) > 0){
                    printf("%s\n",buff);
                    memset(&buff,0,4096);
                }
            }
        }
    }
}
