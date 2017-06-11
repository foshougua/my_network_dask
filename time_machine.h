/*************************************************************************
	> File Name: time_machine.h
	> Author: 
	> Mail: 
	> Created Time: 2017年05月29日 星期一 11时44分55秒
 ************************************************************************/

#ifndef _TIME_MACHINE_H
#define _TIME_MACHINE_H
#include<string>
#include<cstdio>
#include<assert.h>
#include<cstring>
#include"redis_operator.h"
#include"net.h"

class TimeMachine{
public:
    TimeMachine(std::string user_id,std::string file_name,int back_number):user_id_(user_id),file_name_(file_name),back_number_(back_number){}
    ~TimeMachine();
    void SendRequest(int sockfd);//发送请求 用户名+文件名+回退几步
private:
    void Sendn(int sockfd,char *buff,int size,int flags);
    int JudgeNumberBit(int number);
private:
    std::string user_id_;
    std::string file_name_;
    int back_number_;
};


void TimeMachine::Sendn(int sockfd,char *buff,int size,int flags){
    char *temp = buff;
    int result = size;
    while(result > 0){
        int recv_num = send(sockfd,temp,result,flags);
        assert(recv_num != -1);
        result -= recv_num;
        temp += recv_num;
    }
}

int TimeMachine::JudgeNumberBit(int number){
    int result = 0;
    while(number){
        result++;
        number /= 10;
    }
    return result;
}

TimeMachine::~TimeMachine(){}

void TimeMachine::SendRequest(int sockfd){
    char buff[1024];
    char id_and_file[1024];
    //里面发送user_id 和 file_name 以及 back_number
    sprintf(id_and_file,"$%ld\r\n%s\r\n%ld\r\n%s\r\n%d\r\n",user_id_.size(),user_id_.c_str(),file_name_.size(),file_name_.c_str(),back_number_);

    //下一次需要读取的长度5位
    int id_and_file_size = strlen(id_and_file);
    int zero_bit = 5 - JudgeNumberBit(id_and_file_size);//先求出零的位数
    std::string next_recv_size_str;
    while(zero_bit > 0){
        next_recv_size_str.push_back('0');
        zero_bit--;
    }
    next_recv_size_str += std::to_string(id_and_file_size);
    //首先发送请求命令40位
    sprintf(buff,"*05\r\n$04\r\nback\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\r\n$%s\r\n",next_recv_size_str.c_str());
    printf("buff:%s",buff);
    Sendn(sockfd,buff,strlen(buff),0);
    //接着发送用户名、文件名、回退几步
    printf("id_and_file:%s",id_and_file);
    Sendn(sockfd,id_and_file,strlen(id_and_file),0);
    return;
}

#endif
