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
#include<cstring>

class TimeMachine{
public:
    TimeMachine(std::string user_id,std::string file_name,int back_number):user_id_(user_id),file_name_(file_name),back_number_(back_number){}
    ~TimeMachine();
    void SendRequest(int sockfd);//发送请求 用户名+文件名+回退几步
private:
    std::string user_id_;
    std::string file_name_;
    int back_number_;
};

TimeMachine::~TimeMachine(){}

void TimeMachine::SendRequest(){
    char buff[4096];
    sprintf(buff,"*4\r\n$4\r\nback\r\n$%d\r\n%s\r\n$%d\r\n%s\r\n%d\r\n",strlen(user_id_.c_str()),user_id_.c_str(),strlen(file_name_.c_str()),file_name_.c_str(),back_number_);
    if(send(sockfd,buff,strlen(buff),0) == -1){
        std::cerr<<"send time reques throw error!"<<std::endl;
        throw;
    }
}

#endif
