/*************************************************************************
	> File Name: epoll.h
	> Author: 
	> Mail: 
	> Created Time: 2017年05月14日 星期日 10时09分14秒
 ************************************************************************/

#ifndef _EPOLL_H
#define _EPOLL_H
#include<cstdlib>
#include<iostream>
#include<sys/epoll.h>
#include<exception>
#include<stdexcept>
#include"config.h"

class EpollOperate{
public:
    int CreateEpoll(int size = 20);
    void Addfd(int epfd,int fd);
    void Deletefd(int epfd,int fd,struct epoll_event *event);
    void Modifyfd(int epfd,int fd,struct epoll_event *event);
    int Wait(int epfd,struct epoll_event *events,int maxevents = 1024,int timeout = -1);
};

//创建epoll
int EpollOperate::CreateEpoll(int size){
    try{
        int epfd = epoll_create(size);
        if(epfd == -1)
            throw std::exception();
        return epfd;
    }
    catch(...){
        std::cerr<<"create epoll fail!"<<std::endl;
        exit(1);
    }
}

//添加文件描述符
void EpollOperate::Addfd(int epfd,int fd){
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&event);
}

//删除文件描述符
void EpollOperate::Deletefd(int epfd,int fd,struct epoll_event *event){
    try{
        int result = epoll_ctl(epfd,EPOLL_CTL_DEL,fd,event);
        if(result == -1)
            throw std::exception();
    }
    catch(...){
        std::cerr<<"del fail!"<<std::endl;
        exit(1);
    }
}

//修改文件描述符
void EpollOperate::Modifyfd(int epfd,int fd,struct epoll_event *event){
    try{
        int result = epoll_ctl(epfd,EPOLL_CTL_MOD,fd,event);
        if(result == -1)
            throw std::exception();
    }
    catch(...){
        std::cerr<<"mod fail!"<<std::endl;
    }
}

//等待到来的请求
int EpollOperate::Wait(int epfd,struct epoll_event *events,int maxevents,int timeout){
    try{
        int result = epoll_wait(epfd,events,maxevents,timeout);
        if(result == -1)
            throw std::exception();
        return result;
    }
    catch(...){
        std::cerr<<"wait fail!"<<std::endl;
        exit(1);
    }
}

#endif
