/*************************************************************************
	> File Name: file.h
	> Author: 
	> Mail: 
	> Created Time: 2017年05月23日 星期二 14时15分51秒
 ************************************************************************/

#ifndef _FILE_H
#define _FILE_H
#include"config.h"
#include<string>
#include<fstream>

class File{
public:
    File():file_name("test.txt"){}
    File(std::string name):file_name(name){}

    void open_file(){
        if(!file.is_open())
            file.open(file_name,std::ios::binary|std::ios::in);
    }

    std::ifstream &get_file(){
        return file;
    }

    int get_file_size(){
        std::ifstream in(file_name);
        in.seekg(0,std::ios::end);
        int size = in.tellg();
        in.seekg(0,std::ios::beg);
        return size;
    }

    int get_block_number(int size = BLOCK_SIZE){
        int block_number = get_file_size()/size + (get_file_size()%size ? 1:0);
        return block_number;
    }

    void close_file(){
        if(file.is_open())
        {
            std::cout<<"close file"<<std::endl;
            file.close();
        }
    }
    ~File(){
        close_file();
    }

private:
    std::ifstream file;
    std::string file_name;
};
#endif
