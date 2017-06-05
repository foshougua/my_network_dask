/*************************************************************************
	> File Name: upload_file.h
	> Author: 
	> Mail: 
	> Created Time: 2017年05月29日 星期一 10时36分08秒
 ************************************************************************/

#ifndef _UPLOAD_FILE_H
#define _UPLOAD_FILE_H
//#include<sys/types>
#include<sys/socket.h>
#include<assert.h>
#include<cstring>
#include<string>
#include<cstdio>
#include<vector>
#include<unordered_map>
#include"file.h"
#include"net.h"
#include"task.h"
#include"thread_pool.h"
#include"redis_operator.h"

class UploadFile{
public:
    //这里要将文件切割开，并初始化map
    UploadFile(std::string user_id,std::string file_name);
    ~UploadFile();//销毁所有创建的文件
    void SendRequest(int sockfd);//发送文件名+分块列表，主线程内完成
    std::vector<std::string> ReciveBlockList(int sockfd);//接受服务器端返回的分块列表，存储在vector中
    void SendBlockFile(int sockfd,std::vector<std::string> &block_list,std::string server_ip,int port);//根据传回的分块列表，利用多线程发送每个块
private:
    int Sendn(int sockfd,char *buff,int size,int flags);
    int Recvn(int sockfd,char *buff,int size,int flags);
    int JudgeNumberBit(int number);
private:
    std::string user_id_;
    std::string file_name_;
    //存放文件名和md5的对应
    std::unordered_map<std::string,std::string> md5_and_block;
    //有顺序的存放md5，供传送块的使用
    std::vector<std::string> md5_and_block_vecotor;
};

int UploadFile::Sendn(int sockfd,char *buff,int size,int flags){
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

int UploadFile::Recvn(int sockfd,char *buff,int size,int flags){
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

//判断一个数是多少位
int UploadFile::JudgeNumberBit(int number){
    int result = 0;
    while(number){
        result++;
        number /= 10;
    }
    return result;
}

//初始化map和vector
UploadFile::UploadFile(std::string user_id,std::string file_name){
    this->user_id_ = user_id;
    this->file_name_ = file_name;

    std::string dir("temp//");
    OperateFile op;

    int block_number = op.GetBlockNumber(file_name_);

    op.SplitFile(file_name_,dir);

    for(int i = 1; i <= block_number; i++){
        std::string file_name = dir + file_name_ + std::to_string(i);
        std::string md5 = op.GetFileMd5(file_name);
        md5_and_block[md5] = file_name;
        md5_and_block_vecotor.push_back(md5);
    }
}

//删除所有分块的文件
UploadFile::~UploadFile(){
    OperateFile op;
    std::unordered_map<std::string,std::string>::iterator it;
    for(it = md5_and_block.begin();it != md5_and_block.end(); it++){
        std::cout<<"~UploadFile: md5 = "<<it->first<<"\t"<<"file_name = "<<it->second<<std::endl;
        std::string file_name = it->second;
        op.DeleteFile(file_name);
    }
}

//上传请求
//首先发送固定长度的方法体（40字节）
//之后再发送分块列表
void UploadFile::SendRequest(int sockfd){

    char buff[2048];
    char id_and_file[2048];
    char next_send_size[5];

    //提前计算它是因为要将它的长度写入buff进行传送
    sprintf(id_and_file,"$%ld\r\n%s\r\n$%ld\r\n%s\r\n",user_id_.size(),user_id_.c_str(),file_name_.size(),file_name_.c_str());
    printf("client send user_id and file_name:%s\n",id_and_file);
    printf("id_and_file:%s",id_and_file);
    int id_and_file_size = strlen(id_and_file);
    if(id_and_file_size < 10){
        sprintf(next_send_size,"0000%d",id_and_file_size);
    }
    else if(id_and_file_size < 100){
        sprintf(next_send_size,"000%d",id_and_file_size);
    }
    else if(id_and_file_size < 1000){
        sprintf(next_send_size,"00%d",id_and_file_size);
    }
    else if(id_and_file_size < 10000){
        sprintf(next_send_size,"0%d",id_and_file_size);
    }
    else{
        sprintf(next_send_size,"%d",id_and_file_size);
    }

    /*这里传送是固定大小的40个字节，服务器在接受到之后，只需先拿出40个字节来判断是哪一种方法，同时后面的
    next_send_size代表服务器下一次要取的字符个数*/
    sprintf(buff,"*01\r\n$03\r\nput\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\r\n$%s\r\n",next_send_size);

    Sendn(sockfd,buff,strlen(buff),0);
    Sendn(sockfd,id_and_file,strlen(id_and_file),0);

    //将md5的总数量发送出去
    //总数量占8位，每次服务器端读取的时候先把这八个字节的长度读出来，再进行操作
    //不够8位就进行补0
    int zero_bit = 8 - JudgeNumberBit(md5_and_block.size());//先求出零的位数
    std::string md5_and_block_size;
    while(zero_bit > 0){
        md5_and_block_size.push_back('0');
        zero_bit--;
    }
    md5_and_block_size += std::to_string(md5_and_block.size());

    sprintf(buff,"$%s\r\n",md5_and_block_size.c_str());
    printf("md5_and_block_size = %s",buff);
    Sendn(sockfd,buff,strlen(buff),0);

    //一个接一个发送分块列表
    for(auto it = md5_and_block_vecotor.begin();it != md5_and_block_vecotor.end();++it){
        sprintf(buff,"*%s\r\n",(*it).c_str());
        printf("send block list:%s",buff);
        Sendn(sockfd,buff,strlen(buff),0);
    }
}

//接受到传回的分块列表
std::vector<std::string> UploadFile::ReciveBlockList(int sockfd){
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

//根据接受到的分块列表
//将每一个小块文件通过线程池发送出去
//发送sendpieces+md5+文件名
void UploadFile::SendBlockFile(int sockfd,std::vector<std::string> &block_vector,std::string server_ip,int port){
    std::vector<std::string>::iterator it;
    ThreadPool pool(10);

    for(it = block_vector.begin();it != block_vector.end(); ++it){
        std::string file_name = md5_and_block[*it];
        std::cout<<"sendpieces file_name = "<<file_name<<std::endl;
        Task task;
        //task.process(file_name,server_ip,port);
        pool.addTask(std::bind(&Task::process,&task,file_name,server_ip,port));
    }

    while(1){    
        if (pool.size() == 0){  
            pool.stop();
            printf("Now I will exit from main\n"); 
            return;
        }  
        sleep(2);
    }
}

#endif
