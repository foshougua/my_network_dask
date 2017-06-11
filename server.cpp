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
#include<fcntl.h>
#include<sys/epoll.h>
#include<cstdlib>
#include"thread_pool.h"
#include"epoll.h"
#include"config.h"
#include"handle_request.h"

void addfd(int epfd,int fd){
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN|EPOLLONESHOT;
    epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&event);
}

void set_nonblocking(int sockfd){
    int opts;
    opts = fcntl(sockfd,F_GETFL);
    assert(opts > 0);
    opts = opts|O_NONBLOCK;
    assert(fcntl(sockfd,F_SETFL,opts) > 0);
}

int main(int argc,char **argv){

    if(argc < 2){
        std::cerr<<"need input server port!"<<std::endl;
        exit(1);
    }

    char buff[BUFF_SIZE];
    const char *ip = "127.0.0.1";
    int port = atoi(argv[1]);
    printf("server wait:ip = %s,port = %d\n",ip,atoi(argv[1]));

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
    //struct epoll_event events[1024];
    //int epfd = epoll_create(10);
    //addfd(epfd,listenfd);//添加监听描述符

    int ret = 0;
    ret = bind(listenfd,(struct sockaddr *)&server_address,sizeof(server_address));//绑定
    if(ret == -1){
        std::cerr<<"bind error!"<<std::endl;
        exit(1);
    }
    ret = listen(listenfd,10);//监听
    std::cout<<"listen success!--------listend success"<<std::endl;

    if(ret == -1){
        std::cerr<<"listen error!"<<std::endl;
        exit(1);
    }
    ThreadPool pool;
    while(true){
        /*std::cout<<"epoll_wait begin-------------------------------"<<std::endl;
        int ret_count = epoll_wait(epfd,events,1024,-1);
        std::cout<<"epoll_wait after-------------------------------"<<std::endl;

        for(int i = 0; i < ret_count; i++){
            std::cout<<"epoll wait select"<<std::endl;
            int sockfd = events[i].data.fd;
            if(sockfd == listenfd){
                std::cout<<"hah accept listenfd!"<<std::endl;
                struct sockaddr_in client_address;
                socklen_t client_address_len = sizeof(client_address);
                int connfd = accept(listenfd,(struct sockaddr *)&client_address,&client_address_len);
                std::cout<<"add sockfd"<<std::endl;
                addfd(epfd,connfd);
            }
            else if(events[i].events & EPOLLIN){
                std::cout<<"hah accept EPOLLIN!"<<std::endl;
                //HandleRequest hand;
                //hand.DistributeEvent(sockfd);
                pool.addTask(std::bind(&HandleRequest::DistributeEvent,&hand,sockfd));
            }
            else{
                std::cout<<"has other fail!"<<std::endl;
                exit(1);
            }
        }*/
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);
        int sockfd = accept(listenfd,(struct sockaddr *)&client_address,&client_address_len);
        HandleRequest hand;
        pool.addTask(std::bind(&HandleRequest::DistributeEvent,&hand,sockfd));
        //hand.DistributeEvent(connect_fd);
    }
}

/*#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

void setnonblocking(int sock)
{
	int opts;
	opts = fcntl(sock, F_GETFL);

	if(opts < 0) {
		perror("fcntl(sock, GETFL)");
		exit(1);
	}

	opts = opts | O_NONBLOCK;

	if(fcntl(sock, F_SETFL, opts) < 0) {
		perror("fcntl(sock, SETFL, opts)");
		exit(1);
	}
}

int main(int argc, char *argv[])
{
	printf("epoll socket begins.\n");
	ssize_t n;
	char line[1024];
	socklen_t clilen;

	struct epoll_event ev, events[20];

	int epfd = epoll_create(256);

	struct sockaddr_in clientaddr;
	struct sockaddr_in serveraddr;

	int listenfd = socket(AF_INET, SOCK_STREAM, 0);

	setnonblocking(listenfd);

	ev.data.fd = listenfd;
	ev.events = EPOLLIN;

	epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);

	memset(&serveraddr,0,sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	const char *local_addr = "127.0.0.1";
	inet_pton(AF_INET,local_addr, &(serveraddr.sin_addr));
	serveraddr.sin_port = htons(atoi(argv[1]));
	bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	listen(listenfd, 5);

	for(; ;) {
		int nfds = epoll_wait(epfd, events, 20, -1);
		for(int i = 0; i < nfds; ++i) {
			if(events[i].data.fd == listenfd) {
				printf("accept connection, fd is %d\n", listenfd);
				int connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clilen);
				if(connfd < 0) {
					perror("connfd < 0");
					exit(1);
				}

				setnonblocking(connfd);

				char *str = inet_ntoa(clientaddr.sin_addr);
				printf("connect from %s\n", str);

				ev.data.fd = connfd;
				ev.events = EPOLLIN;
				epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
			}
			else if(events[i].events & EPOLLIN) {
                printf("has epoll in!\n");
                int sockfd;
                setnonblocking(sockfd);
				if((sockfd = events[i].data.fd) < 0)
                    continue;
				if((n = read(sockfd, line, 1024)) < 0) {
					if(errno == ECONNRESET) {
						close(sockfd);
						events[i].data.fd = -1;
					} else {
						printf("readline error");
					}
				} else if(n == 0) {
					close(sockfd);
					events[i].data.fd = -1;
				}
               HandleRequest hand;
               hand.DistributeEvent(sockfd);
			}
		}
	}
}*/
