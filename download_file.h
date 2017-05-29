/*************************************************************************
	> File Name: download_file.h
	> Author: 
	> Mail: 
	> Created Time: 2017年05月29日 星期一 11时18分46秒
 ************************************************************************/

#ifndef _DOWNLOAD_FILE_H
#define _DOWNLOAD_FILE_H
#include<sys/types>
#include<sys/socket>
#include<string>
#include<vector>
#include<map>
#include<cstdio>
#include<cstring>

class DownloadFile{
public:
    DownloadFile(std::string user_id,std::string file_name_):user_id_(user_id),file_name_(file_name){}
    ~DownloadFile(){}
    void SendRequest(int sockfd);//发送 用户名+文件名，服务器受到后，会返回一个分块列表
    std::map<int,std::string> ReciveBlockList(int sockfd);//接受这个循环列表,返回 < 偏移量，md5 >
    void DownloadBlockFile(std::map<int,std::string> &block_list);//根据得到的循环列表，多线程请求每个块,并存储
    void MergeBlock();//合并得到的文件
private:
    std::string user_id_;
    std::string file_name_;
};

DownloadFile::~DownloadFile(){
    
}


void DownloadFile::SendRequest(int sockfd){
    char buff[4096];
    sprintf(buff,"*3\r\n$8\r\ndownload\r\n$%d\r\n%s\r\n$%d\r\n%s\r\n",strlen(user_id_.c_str()),user_id_.c_str(),strlen(file_name.c_str()),file_name_.c_str());
    if(send(sockfd,buff,strlen(buff),0) == -1){
        std::cerr<<"download send file name throw error!"<<std::endl;
        throw;
    }

}

std::map<int,std::string> DownloadFile::ReciveBlockList(int sockfd){
    
}

void DownloadFile::DownloadBlockFile(std::map<int,std::string> &block_list){
    
}


//合并文件
void DownloadFile::MergeBlock(){
    return;
}
#endif
