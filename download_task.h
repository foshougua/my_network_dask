/*************************************************************************
	> File Name: download_task.h
	> Author: 
	> Mail: 
	> Created Time: 2017年06月06日 星期二 11时41分44秒
 ************************************************************************/

#ifndef _DOWNLOAD_TASK_H
#define _DOWNLOAD_TASK_H
#include<fstream>
#include"file.h"
#include"net.h"

class DownloadTask{
public:
    void process(std::string file_name,int offset,std::string md5,std::string server_ip_,int server_port_){
        char buff[4096];
        //新创建一个socket，在这个块传输完成之后关闭(析构函数)
        Client cli;
        cli.create_socket();
        cli.connect_to_server(server_ip_,server_port_);
        int sockfd = cli.get_fd();

        //首先发送请求描述符
        sprintf(buff,"*04\r\n$14\r\ndownloadpieces\t\t\t\t\t\t\r\n$00035\r\n");//40位
        Sendn(sockfd,buff,strlen(buff),0);
        //接下来发送md5
        sprintf(buff,"*%s\r\n",md5.c_str());//35位
        printf("file md5:%s",buff);
        Sendn(sockfd,buff,strlen(buff),0);
        //接受该文件的大小
        Recvn(sockfd,buff,11,0);
        std::string file_size_str;
        int file_size;
        for(int i = 1; buff[i] != '\r'; i++){
            file_size_str.push_back(buff[i]);
        }
        std::cout<<"file_size_str = "<<file_size_str<<std::endl;
        file_size = std::stoi(file_size_str);
        memset(buff,0,sizeof(buff));
        //新建一个文件，并把接收到的文件存入文件(client_receive目录下)当中
        OperateFile file_op;
        std::string create_file_name = std::string("client_receive//") + file_name + std::to_string(offset);
        file_op.CreateFile(create_file_name);
        //打开文件
        std::ofstream in;
        in.open(create_file_name,std::ios::out|std::ios::binary);
        //开始接受数据并写入文件
        int total = file_size;
        int temp = 0;
        while(1){
            if(total > 4096)
                temp = 4096;
            else
                temp = total;
            std::cout<<"total = "<<total<<std::endl;
            total -= temp;
            Recvn(sockfd,buff,temp,0);//从socket中读取文件
            printf("%s\n",buff);
            in.write(buff,temp);//将读取的文件写入文件
            memset(buff,0,sizeof(buff)/sizeof(char));
            if(total <= 0){
                in.close();
                return;
            }
        }
    }

private:
    int Sendn(int sockfd,char *buff,int size,int flags);
    int Recvn(int sockfd,char *buff,int size,int flags);
    int JudgeNumberBit(int number);
};

int DownloadTask::Sendn(int sockfd,char *buff,int size,int flags){
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

int DownloadTask::Recvn(int sockfd,char *buff,int size,int flags){
    char *temp = buff;
    int result = size;
    while(result > 0){
        int recv_num = recv(sockfd,temp,result,flags);
        assert(recv_num != -1);
        result -= recv_num;
        temp += recv_num;
    }
    return size;
}

int DownloadTask::JudgeNumberBit(int number){
    int result = 0;
    while(number){
        result++;
        number /= 10;
    }
    return result;
}

#endif
