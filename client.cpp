/*************************************************************************
	> File Name: client.cpp
	> Author: 
	> Mail: 
	> Created Time: 2017年05月13日 星期六 12时24分20秒
 ************************************************************************/

#include<iostream>
#include<vector>
#include<sys/stat.h>
#include"file.h"
#include"config.h"
#include"net.h"
#include"thread_pool.h"
#include"task.h"


int main(int argc,char **argv){
    if(argc < 2){
        std::cerr<<"need input server port!";
        exit(1);
    }

    thread_pool<Task*> p(10);
    p.start();
    File f("test.txt");
    int block_number = f.get_block_number();
    std::cout<<"block_number = "<<block_number<<std::endl;
    for(int i = 1;i <= block_number;i++){
        Task task("test.txt",i,BLOCK_SIZE,"127.0.0.1",atoi(argv[1]));
        //task.process();
        p.add_task(&task);
    }
}
