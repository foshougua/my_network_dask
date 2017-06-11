/*************************************************************************
	> File Name: file.h
	> Author: 
	> Mail: 
	> Created Time: 2017年05月23日 星期二 14时15分51秒
 ************************************************************************/

#ifndef _FILE_H
#define _FILE_H
#include<string>
#include<fstream>
#include<cstdio>
#include<cstring>
#include"config.h"
#include"md5.h"

//默认目录都是temp
class OperateFile{
public:
    OperateFile() = default;
    ~OperateFile();
    int GetFileSize(std::string file_name);
    int GetBlockNumber(std::string file_name);

    bool CreateFile(std::string file_name);
    bool DeleteFile(std::string file_name);
    bool AlterFileName(std::string old_file_name,std::string new_file_name);
    void SplitFile(std::string file_name,std::string dir = std::string("temp//"));
    std::string GetFileMd5(std::string file_name);
private:
    bool IsExitFile(std::string file_name);
};

//析构函数什么都不做
OperateFile::~OperateFile(){
    
}


//获取文件大小
//这里为指定目录，只是求某个文件的大小
int OperateFile::GetFileSize(std::string file_name){
    if(IsExitFile(file_name)){
        std::ifstream in(file_name);
        in.seekg(0,std::ios::end);
        int size = in.tellg();
        return size;
    }
    else{
        std::cerr<<"get file size: the file not exit!"<<std::endl;
        return 0;
    }
}


//获取文件的分块数
//取默认块大小
int OperateFile::GetBlockNumber(std::string file_name){
    if(IsExitFile(file_name)){
        int file_size = GetFileSize(file_name);
        int block_number = file_size/BLOCK_SIZE + (file_size%BLOCK_SIZE ? 1:0);
        return block_number;
    }
    else{
        std::cout<<"file_name = "<<file_name<<std::endl;
        std::cerr<<"get Block number:the file not exit!"<<std::endl;
        return 0;
    }
}

//创建一个文件
bool OperateFile::CreateFile(std::string file_name){
    if(!IsExitFile(file_name)){
        std::ofstream file;
        file.open(file_name,std::ios::out);
        if (!file){
            std::cerr<<"create file error!"<<std::endl;
            return false;
        }
        file.close();
        return true;
    }
}

//删除dir目录下的一个文件
bool OperateFile::DeleteFile(std::string file_name){
    if(IsExitFile(file_name)){
        if (-1 == remove(file_name.c_str())){
            std::cout<<"remove file error!"<<std::endl;
            return false;
        }
    }
    return true;
}

//重命名文件
//旧文件在dir目录下
bool OperateFile::AlterFileName(std::string old_file_name,std::string new_file_name){
    std::string path = old_file_name;
    std::string new_path = new_file_name;
    if(IsExitFile(path)){
        if (-1 == rename(path.c_str(),new_path.c_str())){
            std::cerr<<"rename file error!"<<std::endl;
            return false;
        }
    }
    return true;
}


//将指定的文件按BLOCK_SIZE大小切割成小块
//新创建的文件放入dir目录下
void OperateFile::SplitFile(std::string file_name,std::string dir){ 

    int block_number = GetBlockNumber(file_name);
    std::ifstream in;
    in.open(file_name,std::ios::binary);

    for(int i = 1; i <= block_number; i++){

        in.seekg((i - 1)*BLOCK_SIZE,std::ios::beg);

        //新的文件在dir目录下，同时以源文件+数字为新文件的名字
        std::string new_file =dir + file_name + std::to_string(i);
        if(!CreateFile(new_file)){
            return;
        }

        //打开刚新创建的文件
        std::ofstream out;
        out.open(new_file,std::ios::binary);
        if(out){
            std::cout<<"open success"<<std::endl;
        }
        else{
            std::cout<<file_name;
            std::cout<<"open fail"<<std::endl;
        }
        int total = 0;
        int temp = 0;
        char buff[BUFF_SIZE];

        if(i < block_number){
            total = BLOCK_SIZE;
        }
        else{
            total = GetFileSize(file_name) - in.tellg();
        }

        while(1){
            if(total > BUFF_SIZE){
                temp = BUFF_SIZE;
            }
            else{
                temp = total;
            }
            std::cout<<"total = "<<total<<std::endl;
            total -= temp;

            in.read(buff,temp);
            out.write(buff,temp);

            if(total <= 0){
                out.close();
                break;
            }
        }
    }
    in.close();
}

//判断文件是否存在
//没有指定目录
bool OperateFile::IsExitFile(std::string file_name){
    std::string path = file_name;
    std::ifstream in(path);
    if (in){
        return true;            
    }
    return false;
}

//获取文件的md5码
std::string OperateFile::GetFileMd5(std::string file_name){
    CMessageDigestAlgorithm5 m5;
    std::ifstream in(file_name);
    return m5.Encode(in);
}

#endif
