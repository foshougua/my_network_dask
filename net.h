/*************************************************************************
	> File Name: net.cpp
	> Author: 
	> Mail: 
	> Created Time: 2017年05月13日 星期六 09时36分15秒
 ***********************************************************************/
#ifndef _NET_H_
#define _NET_H_
#include<sys/socket.h>//sockaddr
#include<netinet/in.h>//in_addr,sockaddr_in
#include<unistd.h>
#include<arpa/inet.h>
#include<stdexcept>
#include<exception>
#include<cstdlib>
#include<cstring>
#include<string>
#include"config.h"


//作为基类让下面的clien和server继承
//初始化是即已经创建了它
class Socket{
public:
    Socket():family(AF_INET),type(SOCK_STREAM),protocol(0),fd(-1){}
    Socket(int f,int t,int p):family(f),type(t),protocol(p),fd(-1){}

    //创建socket,返回描述符fd
    void create_socket(){
        try{
            fd = socket(family,type,protocol);
            if(fd == -1)
            {
                std::cerr<<"creat socket throw error!"<<std::endl;
                throw(std::exception());
            }
        }
        catch(std::exception b){
            std::cerr<<"create socket fail!"<<std::endl;
            exit(1);
        } 
    }

    //获取描述符
    int get_fd(){
        return fd;
    }

    virtual ~Socket(){
        try{
            if(close(fd) == -1){
                std::cerr<<"close throw error!"<<std::endl;
                throw (std::exception());
            }
        }
        catch(...){
            exit(1);
        }
    }

protected:
    int family;
    int type;
    int protocol;
    int fd;
};

//客户端类
//继承了Socket
class Client:public Socket{
public:
    //默认构造函数
    Client():Socket(){}

    //传入协议族，套接字类型，和协议
    Client(int f,int t,int p):Socket(f,t,p){}

    //connect传入服务器端的ip和端口号
    //connect之后就可以进行通信了
    void connect_to_server(std::string server_ip,int port){
        try{
            char *ip = new (char)(server_ip.size());
            strcpy(ip,server_ip.c_str());

            struct sockaddr_in server_address;
            memset(&server_address,0,sizeof(struct sockaddr_in));
            server_address.sin_family = AF_INET;
            server_address.sin_port = htons(port);

            if(inet_pton(AF_INET,ip,&server_address.sin_addr) == 0){
                std::cerr<<"inet_pton fail!"<<std::endl;
                throw (std::exception());
            };
            printf("server ip = %s,server port = %d\n",ip,port);

            socklen_t addrlen = sizeof(struct sockaddr);
            if(connect(fd,(struct sockaddr *)&server_address,addrlen) == -1){
                std::cout<<"connect_to_server fail!"<<std::endl;
                throw(std::exception()); 
            }
        }
        catch(std::exception b){
            exit(1);
        }
    }

    //析构函数，什么都不做
    ~Client(){}
};

//服务端类
class Server:public Socket{
public:
    Server(){
        this->ip = std::string(SERVER_IP);
        this->port = SERVER_PORT;
        this->backlog = 1024;
        accept_fd = -1;
    }

    Server(std::string s_ip,int p){
        this->ip = s_ip;
        this->port = p;
        backlog = 1024;
        accept_fd = -1;
    }

    Server(std::string s_ip,int p,int b){
        this->ip = s_ip;
        this->port = p;
        this->backlog = b;
        accept_fd = -1;
    }

    Server(int f,int t,int p,std::string s_ip,int server_port,int b):Socket(f,t,p),ip(s_ip),port(server_port),backlog(b),accept_fd(-1){}

    //绑定并监听
    void bind_to_system(){
        try{
            char *server_ip = new (char)(ip.size());
            strcpy(server_ip,ip.c_str());

            struct sockaddr_in server_address;
            memset(&server_address,0,sizeof(struct sockaddr_in));
            server_address.sin_family = AF_INET;
            server_address.sin_port = htons(port);
            if(inet_pton(AF_INET,server_ip,&server_address.sin_addr) == 0){
                std::cerr<<"inet_pton error!"<<std::endl;
                throw (std::exception());
            }
            printf("server ip = %s,server port = %d\n",server_ip,port);

            //命名
            if(bind(fd,(struct sockaddr *)&server_address,sizeof(struct sockaddr)) == -1){
                std::cerr<<"serevr bind throw error!"<<std::endl;
                throw(std::exception());
            }
        }
        catch(std::exception b){
            std::cerr<<"bind_and_listen fail!"<<std::endl;
            exit(1);
        }
    }

    void listen_request(){
        try{
            if(listen(fd,backlog) == -1){
                std::cerr<<"server listen throw error!"<<std::endl;
                throw(std::exception());
            }
        }
        catch(...){
            exit(1);
        }
    }
    int get_listen_fd(){
        return fd;
    }
    //accept
    void server_accept(){
        try{
            accept_fd = accept(fd,NULL,NULL);
            if(accept_fd == -1){
                std::cerr<<"accept fail!"<<std::endl;
                throw(std::exception());
            }
        }
        catch(std::exception b){
            exit(1);
        }
    }

    //获得accept之后的fd
    int get_accept_fd(){
        return accept_fd;
    }

    ~Server(){
        try{
            if(accept_fd != -1){
                if(close(fd) == -1 || close(accept_fd) == -1){
                    std::cout<<"close accept_fd fail!"<<std::endl;
                    throw(std::exception());
                }
            }
        }
        catch(std::exception b){
            exit(1);
        }
    }

private:
    std::string ip;//ip地址
    int port;//端口号
    int backlog;//listen时的参数
    int accept_fd;//accept之后的描述符
};

#endif
