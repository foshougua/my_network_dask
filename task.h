/*************************************************************************
	> File Name: tash.h
	> Author: 
	> Mail: 
	> Created Time: 2017年05月13日 星期六 12时22分35秒
 ************************************************************************/

#ifndef _TASH_H
#define _TASH_H
//#include<sys/types>
#include<sys/socket.h>
#include<fcntl.h>
#include<unistd.h>
#include<cstdio>
#include<cstring>
#include<cstdlib>
#include<iostream>
#include<fstream>
#include<exception>
#include<string>
#include<fstream>
#include"net.h"
#include"config.h"

//该任务要实现的是发送某个文件的block_size大小至tcp连接的描述符fd中
//每一个任务都需要建立一个socket链接，当该任务进行完时，就要关闭该socket链接
class Task{
public: 
    Task(){}
    void process(std::string file_name_,std::string server_ip_,int server_port_){        
        try{
            char buff[4096];
            //新创建一个socket，在这个块传输完成之后关闭(析构函数)
            Client cli;
            cli.create_socket();
            cli.connect_to_server(server_ip_,server_port_);
            int sockfd = cli.get_fd();

            OperateFile file_op;
            //获取文件大小
            int total = file_op.GetFileSize(file_name_);

            //首先发送请求描述符
            sprintf(buff,"*02\r\n$10\r\nsendpieces\t\t\t\t\t\t\t\t\t\t\r\n$00035\r\n");//40位
            Sendn(sockfd,buff,strlen(buff),0);

            //接着先发送文件的md5值
            std::string md5 = file_op.GetFileMd5(file_name_);
            sprintf(buff,"*%s\r\n",md5.c_str());//35位
            printf("file md5:%s",buff);
            Sendn(sockfd,buff,strlen(buff),0);
            memset(buff,0,sizeof(buff));

            //还要发送8个字节的文件大小，用于服务器端接受
            int zero_bit = 8 - JudgeNumberBit(total);//先求出零的位数
            std::string file_size;
            while(zero_bit > 0){
                file_size.push_back('0');
                zero_bit--;
            }
            file_size += std::to_string(total);
            sprintf(buff,"$%s\r\n",file_size.c_str());//11位
            Sendn(sockfd,buff,strlen(buff),0);
            printf("sendpieces--send file size str:%s",buff);
            memset(buff,0,sizeof(buff));

            //下面发送文件
            std::ifstream in(file_name_);
            int temp = 0;
            while(1){
                if(total > 4096)
                    temp = 4096;
                else
                    temp = total;
                std::cout<<"temp = "<<temp<<std::endl;
                total -= temp;
                in.read(buff,temp);//将数据从文件中读出来
                printf("%s\n",buff);
                Sendn(sockfd,buff,temp,0);//再将读入buff中的数据写入sockfd
                memset(buff,0,sizeof(buff));
                if(total <= 0){
                    in.close();
                    return;
                }
            }
        }
        catch(...){
            exit(1);
        }
    }
private:
    int Sendn(int sockfd,char *buff,int size,int flags);
    int Recvn(int sockfd,char *buff,int size,int flags);
    int JudgeNumberBit(int number);
};

int Task::Sendn(int sockfd,char *buff,int size,int flags){
    char *temp = buff;
    int result = size;
    while(result > 0){
        int recv_num = send(sockfd,temp,result,flags);
        assert(recv_num != -1);
        result -= recv_num;
        temp += recv_num;
    }
    return size; 
}

int Task::Recvn(int sockfd,char *buff,int size,int flags){
    char *temp = buff;
    int result = size;
    while(result > 0){
        int recv_num = recv(sockfd,temp,result,flags);
        assert(recv_num != -1);
        result -= recv_num;
        temp += recv_num;
    } 
}

int Task::JudgeNumberBit(int number){
    int result = 0;
    while(number){
        result++;
        number /= 10;
    }
    return result;
}

#endif
