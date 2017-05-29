/*************************************************************************
	> File Name: redis_operate.h
	> Author: 
	> Mail: 
	> Created Time: 2017年05月29日 星期一 08时08分08秒
 ************************************************************************/

#ifndef _REDIS_OPERATOR_H
#define _REDIS_OPERATOR_H
#include<hiredis/hiredis.h>
#include<string>
#include<iostream>

class RedisOperator{
public:
    RedisOperator(std::string server_ip = std::string("127.0.0.1"),int server_port = 6379):server_ip_(server_ip),server_port_(server_port){
        connect_ = redisConnect(server_ip_.c_str(),server_port_);
        if(connect_->err){
            std::cout<<"connect to redis database throw error!"<<std::endl;
            throw;
        }
    }
    ~RedisOperator();
    bool ConnectRedis();
    bool ExitRedis();
    bool SelectDatabase(int database_number);
    bool HashSet(std::string key,std::string field,std::string value);
    std::string HashGet(std::string key,std::string field);
    bool HashExists(std::string key,std::string field);
private:
    redisContext *connect_;
    std::string server_ip_;
    int server_port_;
};

RedisOperator::~RedisOperator(){
    ExitRedis();
}

bool RedisOperator::ConnectRedis(){
    redisContext *connection = redisConnect(server_ip_.c_str(),server_port_);
    if(connection->err){
        redisFree(connection);
        std::cerr<<"connect to redis throw error!"<<std::endl;
        return false;
    }
    return true;
}

bool RedisOperator::ExitRedis(){
    redisFree(connect_);
}

bool RedisOperator::SelectDatabase(int database_number){
    redisReply *result = (redisReply *) redisCommand(connect_,"select %d",database_number);
    if(NULL == result){
        std::cerr<<"select database throw error!"<<std::endl;
        freeReplyObject(result);
        return false;
    }
    freeReplyObject(result);
    return true;
}

bool RedisOperator::HashSet(std::string key,std::string field,std::string value){
    redisReply *result = (redisReply *)redisCommand(connect_,"hset %s %s %s",key.c_str(),field.c_str(),value.c_str());
    if(result == NULL){
        std::cerr<<"hash set throw error!"<<std::endl;
        freeReplyObject(result);
        return false;
    }
    freeReplyObject(result);
    return true;
}

std::string RedisOperator::HashGet(std::string key,std::string field){
    if(!HashExists(key,field)){
        std::cerr<<"hash get throw error:the key not exists!"<<std::endl;
        return "";
    }
    redisReply *result = (redisReply *)redisCommand(connect_,"hget %s %s",key.c_str(),field.c_str());
    if(result == NULL){
        std::cerr<<"hasg get throw error!"<<std::endl;
        freeReplyObject(result);
        return "";
    }
    std::string reply = result->str;
    freeReplyObject(result);
    return reply;
}


bool RedisOperator::HashExists(std::string key,std::string field){
    redisReply *result = (redisReply *)redisCommand(connect_,"exists %s %s",key.c_str(),field.c_str());
    if(result == NULL)
        return false;
    else
        return true;
}

#endif
