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
    std::string HashGetUsersFileVersion(std::string user_id,std::string file_name);
    std::string HashGetUsersFileMd5Number(std::string user_id,std::string file_name,int version);
    std::string HashGetUsersFileMd5(std::string user_id,std::string file_name,int version,int number);
    bool HashDecrbyUsersFileVersion(std::string user_id,std::string file_name,int decry_count);
    bool HashDeleteUsersFileMd5(std::string user_id,std::string file_name,int version,int offset);
    bool HashDeleteUsersFileVersionCount(std::string user_id,std::string file_name,int version);
    bool HashSetUsersVersion(std::string user_id,std::string file_name,int version);
    bool HashSetUsersMd5(std::string user_id,std::string file_name,int version,int number,std::string md5);
    bool HashIncrUsersVersion(std::string user_id,std::string file_name);
    bool HashSetUsersMd5Count(std::string user_id,std::string file_name,int version,int md5_number);
    bool HashIncrMedataMd5Count(std::string md5);
    bool HashExistsUsersFile(std::string user_id,std::string file_name);
    bool HashExistsMedataMd5(std::string md5);
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

std::string RedisOperator::HashGetUsersFileVersion(std::string user_id,std::string file_name){
    std::string key("users");
    std::string field = user_id + ":" + file_name + ":" + std::string("now_version");
    return HashGet(key,field);
}

std::string RedisOperator::HashGetUsersFileMd5Number(std::string user_id,std::string file_name,int version){
    std::string key("users");
    std::string field = user_id + ":" + file_name + ":" + std::to_string(version) + ":count";
    return HashGet(key,field);
}

std::string RedisOperator::HashGetUsersFileMd5(std::string user_id,std::string file_name,int version,int number){
    std::string key("users");
    std::string field = user_id+":"+file_name+":"+std::to_string(version)+":"+std::to_string(number);
    return HashGet(key,field);
}

bool RedisOperator::HashDecrbyUsersFileVersion(std::string user_id,std::string file_name,int decry_count){
    std::string key("users");
    std::string field = user_id + ":" + file_name + ":" + std::string("now_version");
    int count = -decry_count;
    redisReply *result = (redisReply *)redisCommand(connect_,"hincrby %s %s %d",key.c_str(),field.c_str(),count);
    if(result == NULL)
        return false;
    else
        return true;
}

bool RedisOperator::HashDeleteUsersFileMd5(std::string user_id,std::string file_name,int version,int offset){
    std::string key("users");
    std::string field = user_id + ":" + file_name + ":" + std::to_string(version) + ":" + std::to_string(offset);
    redisReply *result = (redisReply *)redisCommand(connect_,"hdel %s %s",key.c_str(),field.c_str());
    if(result->integer == 0)
        return false;
    else
        return true;
}
bool RedisOperator::HashDeleteUsersFileVersionCount(std::string user_id,std::string file_name,int version){
    std::string key("users");
    std::string field = user_id + ":" + file_name + ":" + std::to_string(version) + ":count";
    redisReply *result = (redisReply *)redisCommand(connect_,"hdel %s %s",key.c_str(),field.c_str());
    if(result->integer == 0)
        return false;
    else
        return true;
}

bool RedisOperator::HashSetUsersVersion(std::string user_id,std::string file_name,int version){
    std::string key("users");
    std::string field = user_id + ":" + file_name + ":" + std::string("now_version");
    std::string value = std::to_string(version);
    if(HashSet(key,field,value))
        return true;
    else
        return false;
}

bool RedisOperator::HashSetUsersMd5(std::string user_id,std::string file_name,int version,int number,std::string md5){
    std::string key("users");
    std::string field = user_id + ":" + file_name + ":" + std::to_string(version) + ":" + std::to_string(number);
    std::string value(md5);
    if(HashSet(key,field,value))
        return true;
    else
        return false;
}

bool RedisOperator::HashSetUsersMd5Count(std::string user_id,std::string file_name,int version,int md5_number){
    std::string key("users");
    std::string field = user_id + ":" + file_name + ":" + std::to_string(version) + ":count";
    std::string value = std::to_string(md5_number);
    if(HashSet(key,field,value))
        return true;
    else
        return false;
}

bool RedisOperator::HashIncrUsersVersion(std::string user_id,std::string file_name){
    std::string key("users");
    std::string field = user_id + ":" + file_name + ":" + std::string("now_version");
    redisReply *result = (redisReply *)redisCommand(connect_,"hincrby %s %s 1",key.c_str(),field.c_str());
    if(result == NULL)
        return false;
    else
        return true;
}

bool RedisOperator::HashIncrMedataMd5Count(std::string md5){
    std::string key("medata");
    std::string field(md5);
    redisReply *result = (redisReply *)redisCommand(connect_,"hincrby %s %s 1",key.c_str(),field.c_str());
    if(result == NULL)
        return false;
    else
        return true;
}

bool RedisOperator::HashExistsUsersFile(std::string user_id,std::string file_name){
    std::string key("users");
    std::string field = user_id + ":" + file_name + ":" + std::string("now_version");
    if(HashExists(key,field))
        return true;
    else
        return false;
}

bool RedisOperator::HashExistsMedataMd5(std::string md5){
    std::string key("medata");
    std::string field(md5);
    if(HashExists(key,field))
        return true;
    else
        return false;
}


bool RedisOperator::HashExists(std::string key,std::string field){
    redisReply *result = (redisReply *)redisCommand(connect_,"hexists %s %s",key.c_str(),field.c_str());
    if(result->integer == 0)
        return false;
    else
        return true;       
}

#endif
