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
#include<assert.h>
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
    std::vector<std::string> ReciveBlockList(int sockfd);//接受这个循环列表,返回 < 偏移量，md5 >
    void DownloadBlockFile(std::vector<std::string> &block_list);//根据得到的循环列表，多线程请求每个块,并存储
    void MergeBlock();//合并得到的文件
private:
    int Sendn(int sockfd,char *buff,int size,int flags);
    int Recvn(int sockfd,char *buff,int size,int flags);
    int JudgeNumberBit(int number);
private:
    std::string user_id_;
    std::string file_name_;
};


int DownloadFile::Sendn(int sockfd,char *buff,int size,int flags){
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


int DownloadFile::Recvn(int sockfd,char *buff,int size,int flags){
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

int DownloadFile::JudgeNumberBit(int number){
    int result = 0;
    while(number){
        result++;
        number /= 10;
    }
    return result;
}

DownloadFile::~DownloadFile(){}


void DownloadFile::SendRequest(int sockfd){
    char buff[1024];
    char id_and_file[1024];
    sprintf(id_and_file,"$%ld\r\n%s\r\n$%dl\r\n%s\r\n",user_id_.size(),user_id_.c_str(),file_name_.size(),file_name_.c_str());

    //计算id_and_file的长度，一共5位，不够的补0
    int zero_bit = 5 - JudgeNumberBit(strlen(id_and_file));//先求出零的位数
    std::string id_and_file_size;
    while(zero_bit > 0){
        id_and_file_size.push_back('0');
        zero_bit--;
    }
    id_and_file_size += std::to_string(strlen(id_and_file));

    sprintf(buff,"*03\r\n$08\r\ndownload\t\t\t\t\t\t\t\t\t\t\t\t\r\n$%s\r\n",id_and_file_size.c_str());
    //先发送请求头，40字节
    Sendn(sockfd,buff,strlen(buff),0);
    //接着发送用户名和文件名
    Sendn(sockfd,id_and_file,strlen(id_and_file),0);
}

//接受回复的分块列表
std::vector<std::string> DownloadFile::ReciveBlockList(int sockfd){
    char buff[2048];
    //先接受整个列表的长度
    memset(buff,0,sizeof(buff));
    Recvn(sockfd,buff,11,0);//取出分块列表的个数
    std::string temp;
    for(int i = 1;i <= 8; i++){
        temp.push_back(buff[i]);
    }
    int block_number = std::stoi(temp);//得到分块列表的个数
    std::cout<<"block_number = "<<block_number<<std::endl;
    memset(buff,0,sizeof(buff));
    //根据分块列表的个数，拿出分块列表
    //每次只拿出一个分块列表直到拿完
    std::vector<std::string> md5_block_vector;
    int block_number_temp = block_number;
    while(block_number_temp > 0){
        std::string md5;
        Recvn(sockfd,buff,35,0);
        printf("recv block list:%s",buff);
        for(int i = 1; i <= 32; i++){
            md5.push_back(buff[i]);
        }
        md5_block_vector.push_back(md5);
        block_number_temp--;
    }

    for(auto it = md5_block_vector.begin(); it != md5_block_vector.end(); ++it){
        std::cout<<"md5_block_vector--md5:"<<(*it)<<std::endl;
    }
    return md5_block_vector;
}

void DownloadFile::DownloadBlockFile(std::vector<std::string> &block_list){
    return;
}
//合并文件
void DownloadFile::MergeBlock(){
    return;
}
#endif
