/*************************************************************************
	> File Name: tash.h
	> Author: 
	> Mail: 
	> Created Time: 2017年05月13日 星期六 12时22分35秒
 ************************************************************************/

#ifndef _TASH_H
#define _TASH_H
#include<sys/types>
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
#include<mutex>
#include<fstream>
#include"net.h"
#include"config.h"

//该任务要实现的是发送某个文件的block_size大小至tcp连接的描述符fd中
//每一个任务都需要建立一个socket链接，当该任务进行完时，就要关闭该socket链接

class Task{
public:
    //Task(std::string file_name,std::string server_ip,int server_port):file_name_(file_name),server_ip_(server_ip),server_port_(server_port){}
    
    Task(){}

    void process(std::string file_name_,std::string server_ip_,int server_port_){        
        try{
            char buff[BUFF_SIZE];

            //新创建一个socket，在这个块传输完成之后关闭(析构函数)
            Client cli;
            cli.create_socket();
            cli.connect_to_server(server_ip_,server_port_);
            int sockfd = cli.get_fd();

            //打开文件
            std::ifstream in(file_name_);

            //获取文件大小
            in.seekg(0,std::ios::end);
            int total = in.tellg();
            in.seekg(0,std::ios::beg);

            //先发送给文件的md5值
            CMessageDigestAlgorithm5 m;
            std::string md5 = m.Encode(in);
            sprintf(buff,"*2\r\n$10\r\nsendpieces\r\n$%d\r\n%s\r\n",2,"SENDPIECES",strlen(md5.c_str()),md5.c_str());
            if(send(sockfd,buff,strlen(buff),0) == -1){
                std::cerr<<"send one block error!"<<std::endl;
                throw;
            }
            memset(buff,0,sizeof(buff));

            int temp = 0;
            while(1){

                if(total > BUFF_SIZE)
                    temp = BUFF_SIZE;
                else
                    temp = total;

                total -= temp;

                in.read(buff,temp);//将数据从文件中读出来
                int nwrite = write(sockfd,buff,temp);//再将读入buff中的数据写入sockfd
                if(nwrite == -1){
                    std::cerr<<"write to sockfd throw error!"<<std::endl;
                    throw (std::exception());
                }
                memset(buff,0,sizeof(buff)/sizeof(char));
                std::cout<<"total = "<<total<<std::endl;
                if(total <= 0){
                    in.close();
                    OperateFile op;
                    op.DeleteFile(file_name_);
                    std::cout<<file_name_<<" has delete"<<std::endl;
                    return;
                }
            }
        }
        catch(...){
            exit(1);
        }
    }

    ~Task(){}

/*private:
    std::string file_name_;
    std::string server_ip_;
    int server_port_;*/
};

#endif
