/*************************************************************************
	> File Name: tash.h
	> Author: 
	> Mail: 
	> Created Time: 2017年05月13日 星期六 12时22分35秒
 ************************************************************************/

#ifndef _TASH_H
#define _TASH_H

#include<fcntl.h>
#include<unistd.h>
#include<cstdio>
#include<cstdlib>
#include<iostream>
#include<fstream>
#include<exception>
#include<cstring>
#include<string>
#include<mutex>
#include<fstream>
#include"net.h"
#include"config.h"
#include"file.h"

//该任务要实现的是发送某个文件的block_size大小至tcp连接的描述符fd中
//每一个任务都需要建立一个socket链接，当该任务进行完时，就要关闭该socket链接

class Task{
public:
    Task():in("test.txt"),block(1),size(BLOCK_SIZE),server_ip(SERVER_IP),server_port(SERVER_PORT){}

    Task(std::string file_name,int n_block,int block_size = BLOCK_SIZE,std::string ip = std::string(SERVER_IP),int port = SERVER_PORT):in(file_name),block(n_block),size(block_size),server_ip(ip),server_port(port){}

    void process(){        
        try{
            char buff[BUFF_SIZE];
            //新创建一个socket，在这个块传输完成之后关闭
            Client cli;
            cli.create_socket();
            cli.connect_to_server("127.0.0.1",server_port);
            int sockfd = cli.get_fd();

            in.open_file();//打开文件

            int file_size = in.get_file_size();//文件的总大小
            int block_number = in.get_block_number();//总的块数

            in.get_file().seekg((block - 1)*size,std::ios::beg);
            std::cout<<"location = "<<in.get_file().tellg()<<std::endl;
            long total = 0;
            long temp = 0;

            if(block < block_number){
                total = size;
            }
            else if(block == block_number){
                total = file_size - (block-1)*size;
            }
            else{
                return;
            }

            while(1){
                if(total > BUFF_SIZE)
                    temp = BUFF_SIZE;
                else
                    temp = total;
                total -= temp;
                in.get_file().read(buff,temp);//将数据从文件中读出来
                int nwrite = write(sockfd,buff,temp);//再将读入buff中的数据写入sockfd
                if(nwrite == -1){
                    std::cerr<<"write to sockfd throw error!"<<std::endl;
                    throw (std::exception());
                }
                memset(buff,0,sizeof(buff)/sizeof(char));
                std::cout<<"total = "<<total<<std::endl;
                if(total <= 0){
                    return;
                }
            }
        }
        catch(...){
            exit(1);
        }
    }

    ~Task(){
    }

private:
    std::mutex mut;
    File in;
    int block;
    long size;
    std::string server_ip;
    int server_port;
};

#endif
