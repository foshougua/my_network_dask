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

class Epoll{
public:
    Epoll(int s = 10):size(s),epfd(-1){}

    void create_epoll{
        try{
            epfd = epoll_create(size);
            if(epfd == -1)
                throw std::exception();
        }
        catch(...){
            std::cerr<<"create epoll fail!"<<std::endl;
            exit(1);
        }
    }

    void add(int fd){
        try{
            struct epoll_event event;
            event.events = EPOLLIN;
            event.data.fd = fd;
            int result = epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&event);
            if(result == -1)
                throw std::exception();
        }
        catch(...){
            std::cerr<<"add fail!"<<std::endl;
            exit(1);
        }
    }

    void del(int fd,struct epoll_event *event){
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

    void mod(int fd,struct epoll_event *event){
        try{
            int result = epoll_ctl(epfd,EPOLL_CTL_MOD,fd,event);
            if(result == -1)
                throw std::exception();
        }
        catch(...){
            std::cerr<<"mod fail!"<<std::endl;
        }
    }

    int wait(struct epoll_event *events,int maxevents = 1024,int timeout = -1){
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

private:
    int size;
    int epfd;
};


#endif
