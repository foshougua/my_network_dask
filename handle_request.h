/*************************************************************************
	> File Name: handle_request.h
	> Author: 
	> Mail: 
	> Created Time: 2017年05月29日 星期一 14时41分05秒
 ************************************************************************/

#ifndef _HANDLE_REQUEST_H
#define _HANDLE_REQUEST_H

#include<sys/socket.h>
#include<assert.h>
#include<cstdlib>
#include<cstring>
#include<vector>
#include<string>
#include<fstream>
#include"file.h"
#include"redis_operator.h"

class HandleRequest{
public:
    //负责读取相应的事件类型，进行分发
    void DistributeEvent(int sockfd);

    //下面就是对于各种不同事件的处理
    void ReplyPut(int sockfd,int next_recv_size);
    void ReplySendPieces(int sockfd,int next_recv_size);
    void ReplyDownLoad(int sockfd,int next_recv_size);
    void ReplyDownLoadPieces(int sockf,int next_recv_size);
    void ReplyBack(int sockfd,int next_recv_size);
private:
    int JudgeNumberBit(int number);
private:
    int Sendn(int sockfd,char *buff,int size,int flags);
    int Recvn(int sockfd,char *buff,int size,int flags);
private:
    //定义了事件类型
    enum event_type{PUT = 1,SENDPIECES,DOWNLOAD,BACK,DOWNLOADPIECES};
};

int HandleRequest::Sendn(int sockfd,char *buff,int size,int flags){
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


int HandleRequest::Recvn(int sockfd,char *buff,int size,int flags){
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

int HandleRequest::JudgeNumberBit(int number){
    int result = 0;
    while(number){
        result++;
        number /= 10;
    }
    return result;
}

//拿出传送过来的消息头，用来判断是怎样的事件
void HandleRequest::DistributeEvent(int sockfd){
    enum event_type method;
    char buff[1024];
    Recvn(sockfd,buff,40,0);
    printf("%s\n",buff);
    int method_id = (buff[1] - '0') * 10 + (buff[2] - '0');//消息的id
    printf("method_id = %d\n",method_id);
    int method_length = (buff[6] - '0') * 10 + (buff[7] - '0');//消息的长度
    printf("method_length = %d\n",method_length);
    char *method_string = (char *)malloc(method_length + 1);
    strncpy(method_string,buff+10,method_length);
    method_string[method_length] = '\0';
    printf("method_string = %s\n",method_string);
    char next_recv_size_str[5];
    strncpy(next_recv_size_str,buff+33,5);
    printf("next_recv_size_str = %s\n",next_recv_size_str);
    int next_recv_size = atoi(next_recv_size_str);
    printf("next_recv_size = %d\n",next_recv_size);
    switch(method_id){
        case 1:method = PUT;break;
        case 2:method = SENDPIECES;break;
        case 3:method = DOWNLOAD;break;
        case 4:method = DOWNLOADPIECES;break;
        case 5:method = BACK;break;
        default:printf("method id throw error!");return;
    }
    switch(method){
        case PUT:ReplyPut(sockfd,next_recv_size);break;
        case SENDPIECES:ReplySendPieces(sockfd,next_recv_size);break;
        case DOWNLOAD:ReplyDownLoad(sockfd,next_recv_size);break;
        case DOWNLOADPIECES:ReplyDownLoadPieces(sockfd,next_recv_size);break;
        case BACK:ReplyBack(sockfd,next_recv_size);break;
    }
    return;
}

//要接收到客户端发送的分块列表
//然后回复一个应该上传的分块列表
void HandleRequest::ReplyPut(int sockfd,int next_recv_size){
    char buff[1024];
    Recvn(sockfd,buff,next_recv_size,0);
    printf("buff = %s",buff);
    std::string user_id;
    int user_id_size = 0;
    std::string file_name;
    int file_name_size = 0;
    int i = 0;
    //拿出use_id_size
    while(buff[i] != '\r'){
        if(buff[i] == '$'){
            ++i;
            continue;
        }
        user_id_size = user_id_size * 10 + buff[i] - '0';
        ++i;
    }
    i += 2;//跳过\r\n
    //拿出user_id
    while(buff[i] != '\r'){
        user_id.push_back(buff[i++]);
    }
    i += 2;//跳过\r\n
    //拿出file_name_size
    while(buff[i] != '\r'){
        if(buff[i] == '$'){
            ++i;
            continue;
        }
        file_name_size = file_name_size * 10 + buff[i] - '0';
        ++i;
    }
    i += 2;
    //拿出file_name
    while(buff[i] != '\r'){
        file_name.push_back(buff[i++]);
    }

    //这里拿出接受到的分块列表（固定长度）
    memset(buff,0,sizeof(buff));
    Recvn(sockfd,buff,11,0);//取出分块列表的个数
    std::string temp;
    for(int i = 1;i <= 8; i++){
        temp.push_back(buff[i]);
    }
    int block_number = std::stoi(temp);//得到分块列表的个数
    std::cout<<"block_number = "<<block_number<<std::endl;
    memset(buff,0,sizeof(buff));
    //根据分块列表的个数，从sockfd中拿出分块列表
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

    //取到分块列表之后，先将它存入相对应用户的数据库中
    RedisOperator redis_op;
    int version = 1;
    if(!redis_op.HashExistsUsersFile(user_id,file_name)){
        version = 1;
        redis_op.HashSetUsersVersion(user_id,file_name,version);
    }
    else{
        redis_op.HashIncrUsersVersion(user_id,file_name);
        version = std::stoi(redis_op.HashGet(std::string("users"),user_id+":"+file_name+":"+std::string("now_version")));
    }
    //将md5值存入users数据库中
    //先存入该版本的md5的总数量
    redis_op.HashSetUsersMd5Count(user_id,file_name,version,md5_block_vector.size());
    int md5_sequence = 1;
    for(auto it = md5_block_vector.begin(); it != md5_block_vector.end(); ++it){
        redis_op.HashSetUsersMd5(user_id,file_name,version,md5_sequence,(*it));
        md5_sequence++;
    }
    //接下来就要回复给客户端一个分块列表
    //先判断是否在medata中，如果在，就增加它的引用计数
    //不在的话就将md5添加进medata数据库中
    std::vector<std::string> reply_block_vector;
    for(auto it = md5_block_vector.begin(); it != md5_block_vector.end(); ++it){
        std::cout<<"reply md5 = "<<*it<<std::endl;
        if(redis_op.HashExistsMedataMd5(*it)){
            std::cout<<"the md5 exists in the medata"<<std::endl;
            //redis_op.HashIncrMedataMd5Count(*it);
        }
        else{
            reply_block_vector.push_back(*it);
            std::cout<<"the md5 not exists and store!"<<std::endl;
            redis_op.HashSet(std::string("medata"),*it,std::to_string(1));
        }
    }
    //先发送分块列表长度,总长是8位
    int zero_bit = 8 - JudgeNumberBit(reply_block_vector.size());//先求出零的位数
    std::cout<<"reply_block_vector_size = "<<reply_block_vector.size()<<std::endl;
    std::string reply_block_vector_size;
    while(zero_bit > 0){
        reply_block_vector_size.push_back('0');
        zero_bit--;
    }
    reply_block_vector_size += std::to_string(reply_block_vector.size());

    sprintf(buff,"$%s\r\n",reply_block_vector_size.c_str());
    printf("reply_block_vector_size = %s",buff);
    Sendn(sockfd,buff,strlen(buff),0);
    //一个接一个发送分块列表
    for(auto it = reply_block_vector.begin();it != reply_block_vector.end();++it){
        sprintf(buff,"*%s\r\n",(*it).c_str());
        printf("reply block list:%s",buff);
        Sendn(sockfd,buff,strlen(buff),0);
    }
    return;
}

//具体的接受每一个分块
void HandleRequest::ReplySendPieces(int sockfd,int next_recv_size){
    char buff[4096];
    
    //首先拿出md5值
    Recvn(sockfd,buff,next_recv_size,0);
    std::string md5;
    for(int i = 1; buff[i] != '\r'; i++){
        md5.push_back(buff[i]);
    }
    memset(buff,0,sizeof(buff));

    //接着拿出文件的大小
    Recvn(sockfd,buff,11,0);
    std::string file_size_str;
    int file_size;
    for(int i = 1; buff[i] != '\r'; i++){
        file_size_str.push_back(buff[i]);
    }
    std::cout<<"file_size_str = "<<file_size_str<<std::endl;
    file_size = std::stoi(file_size_str);
    memset(buff,0,sizeof(buff));

    
    //新建一个文件，并把接收到的文件存入文件(accept目录下)当中
    OperateFile file_op;
    std::string file_name = std::string("accept//") + md5;
    file_op.CreateFile(file_name);
    //打开文件
    std::ofstream in;
    in.open(file_name,std::ios::out|std::ios::binary);
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

//处理下载文件
void HandleRequest::ReplyDownLoad(int sockfd,int next_recv_size){
    char buff[2048];
    Recvn(sockfd,buff,next_recv_size,0);
    std::string user_id;
    int user_id_size = 0;
    std::string file_name;
    int file_name_size = 0;
    
    int i = 1;
    while(buff[i] !='\r'){
        //拿出use_id_size
        user_id_size = (user_id_size * 10) + (buff[i++] - '0');
    }
    i += 2;//越过\r\n
    while(buff[i] != '\r'){
        //提取出use_id
        user_id.push_back(buff[i++]);
    }
    i += 2;
    while(buff[i] != '\r'){
        //拿出file_name_size
        file_name_size = (file_name_size * 10) + (buff[i++] - '0');
    }
    i += 2;
    while(buff[i] != '\r'){
        //拿出file_name
        file_name.push_back(buff[i++]);
    }
    //下面根据user_id 和 file_name 拿出分块列表，传送回去
    //先取出当前的历史版本
    //逐次取出这个历史版本中的md5文件名
    RedisOperator redis_op;
    std::vector<std::string> md5_block_vector;//存放所有md5
    int version;//当前版本号
    int sum_of_md5;//md5总数
    version = std::stoi(redis_op.HashGetUsersFileVersion(user_id,file_name));
    sum_of_md5 = std::stoi(redis_op.HashGetUsersFileMd5Number(user_id,file_name,version));
    //拿出所有的md5
    for(int i = 1; i <= sum_of_md5; i++){
        std::string one_md5;
        one_md5 = redis_op.HashGetUsersFileMd5(user_id,file_name,version,i);
        md5_block_vector.push_back(one_md5);
    }
    //先发送md5的个数
    int zero_bit = 8 - JudgeNumberBit(sum_of_md5);//先求出零的位数
    std::string sum_of_md5_str;
    while(zero_bit > 0){
        sum_of_md5_str.push_back('0');
        zero_bit--;
    }
    sum_of_md5_str += std::to_string(sum_of_md5);
    sprintf(buff,"$%s\r\n",sum_of_md5_str.c_str());
    Sendn(sockfd,buff,strlen(buff),0);
    //接着一个接一个的发送md5
    for(auto it = md5_block_vector.begin(); it != md5_block_vector.end(); it++){
        sprintf(buff,"*%s\r\n",(*it).c_str());
        Sendn(sockfd,buff,strlen(buff),0); 
    }
    return;
}

//处理下载的具体的块
void HandleRequest::ReplyDownLoadPieces(int sockfd,int next_recv_size){
    char buff[4096];
    Recvn(sockfd,buff,next_recv_size,0);
    //拿出传送的md5码
    int i = 2;
    std::string md5;
    while(buff[i] != '\r'){
        md5.push_back(buff[i++]);
    }
    RedisOperator redis_op;
    OperateFile file_op;
    //根据md5码，拿出对应的文件 
    //先发送文件的大小
    std::string file_name = std::string("accept//")+md5;//文件名
    int file_size = file_op.GetFileSize(file_name);//文件大小
    int zero_bit = 8 - JudgeNumberBit(file_size);//先求出零的位数
    std::string file_size_str;
    while(zero_bit > 0){
        file_size_str.push_back('0');
        zero_bit--;
    }
    file_size_str += std::to_string(file_size);
    sprintf(buff,"$%s\r\n",file_size_str.c_str());//11位
    Sendn(sockfd,buff,strlen(buff),0);//发送文件大小
    printf("doweloadpieces--send file size str:%s",buff);
    //接着发送整个文件
    std::ifstream in(file_name);
    int temp = 0;
    while(1){
        if(file_size > 4096)
            temp = 4096;
        else
            temp = file_size;
        std::cout<<"temp = "<<temp<<std::endl;
        file_size -= temp;
        in.read(buff,temp);//将数据从文件中读出来
        printf("%s\n",buff);
        Sendn(sockfd,buff,temp,0);//再将读入buff中的数据写入sockfd
        memset(buff,0,sizeof(buff));
        if(file_size <= 0){
            in.close();
            return;
        }
    }
}

//处理时光机请求
void HandleRequest::ReplyBack(int sockfd,int next_recv_size){
    char buff[2048];
    //这里假设next_recv_size的值小于2048
    Recvn(sockfd,buff,next_recv_size,0);
    std::string user_id;
    int user_id_size;
    std::string file_name;
    int file_name_size;
    int back_number = 0;

    int i = 1;
    while(buff[i] !='\r'){
        //拿出use_id_size
        user_id_size = (user_id_size * 10) + (buff[i++] - '0');
    }
    i += 2;//越过\r\n
    while(buff[i] != '\r'){
        //提取出use_id
        user_id.push_back(buff[i++]);
    }
    i += 2;
    while(buff[i] != '\r'){
        //拿出file_name_size
        file_name_size = (file_name_size * 10) + (buff[i++] - '0');
    }
    i += 2;//跨过\r\n
    while(buff[i] != '\r'){
        //拿出file_name
        file_name.push_back(buff[i++]);
    }
    i += 2;//跨过\r\n
    while(buff[i] != '\r'){
        back_number = (back_number * 10) + (buff[i++] - '0');
    }
    //根据得到的文件名、用户名以及回退的步数修改medata库中相对应的文件的当前版本就可以
    RedisOperator redis_op;
    std::string key("medata");
    std::string field = user_id + ":" + file_name + ":" + std::string("version");
    std::string value = std::to_string(back_number);
    redis_op.HashSet(key,field,value);
    return;
}

#endif
