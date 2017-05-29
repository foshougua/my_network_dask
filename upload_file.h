/*************************************************************************
	> File Name: upload_file.h
	> Author: 
	> Mail: 
	> Created Time: 2017年05月29日 星期一 10时36分08秒
 ************************************************************************/

#ifndef _UPLOAD_FILE_H
#define _UPLOAD_FILE_H
#include<sys/types>
#include<sys/socket.h>
#include<cstring>
#include<string>
#include<cstdio>
#include<vector>
#include<map>
#include"file.h"
#include"net.h"
#include"task.h"
#include"thread_pool.h"

class UploadFile{
public:
    //这里要将文件切割开，并初始化map
    UploadFile(std::string user_id,std::string file_name);
    ~UploadFile(){}//销毁所有创建的文件
    void SendRequest(int sockfd);//发送文件名+分块列表，主线程内完成
    std::vector<std::string> ReciveBlockList(int sockfd);//接受服务器端返回的分块列表，存储在vector中
    void SendBlockFile(int sockfd,std::vector<std::string> &block_list,std::strint server_ip,int port);//根据传回的分块列表，利用多线程发送每个块
private:
    std::string user_id_;
    std::string file_name_;
    std::map<std::string,std::string> md5_and_Block;
};

//初始化map
UploadFile::UploadFile(std::string user_id,std::string file_name){
    std::string dir("temp//");
    OperateFile op;
    int block_number = op.GetBlockNumber(file_name_);
    op.SplitFile(file_name_,dir);
    for(int i = 1; i <= block_number; i++){
        std::string file_name = dir + file_name_ + to_string(i);
        std::string md5 = op.GetFileMd5(file_name);
        md5_and_Block[md5] = file_name;
    }
}

//删除所有分块的文件
UploadFile::~UploadFile(){
    OperateFile op;
    std::map<std::string,std::string>::iterator it;
    for(it = md5_and_Block.begin();it != md5_and_Block.end(); it++){
        std::string file_name = it->second;
        op.DeleteFile(file_name);
    }
}

//上传分块列表
void UploadFile::SendRequest(int sockfd){
    char buff[4096];
    //这里是否需要添加md5文件的总数？
    sprintf(buff,"*3\r\n$3\r\nput\r\n$%d\r\n%s\r\n$%d\r\n%s\r\n",strlen(user_id_.c_str()),user_id_.c_str(),strlen(file_name_.c_str()),file_name_.c_str());
    if(send(sockfd,buff,strlen(buff),0) == -1){
        std::cerr<<"send throw error"<<std::endl;
        return;
    }
    std::map<std::string,std::string>::iterator it;
    int i = 1;
    for(it = md5_and_Block.begin();it != md5_and_Block.end();++it){
        sprintf(buff,"$%d\r\n%s\r\n",i++,(it->first).c_str());
        if(send(sockfd,buff,strlen(buff),0) == -1){
            std::cerr<<"send throw error!"<<std::endl;
            throw;
        }
    }
}

//接受到传回的分块列表
std::vector<std::string> UploadFile::ReciveBlockList(int sockfd){
    std::vector<std::string> block_vector;
    char buff[4096];
    while()
}


//每一个小块文件通过线程池发送出去
void UploadFile::SendBlockFile(int sockfd,std::vector<std::string> &block_vector,string server_ip,int port){
    std::vector<std::string>::iterator it;
    ThreadPool pool(10);
    for(it = block_vector.begin();it != block_vector.end(); ++it){
        std::file_name = *it;
        Task task;
        pool.addTask(std::bind(&Task::process,&task,file_name,"127.0.0.1",port);
    }
}

#endif
