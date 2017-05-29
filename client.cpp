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

    OperateFile temp;
    int block_number = temp.GetBlockNumber(std::string("test.txt"));
    std::cout<<block_number<<std::endl;
    std::string dir("temp//");
    temp.SplitFile("test.txt",dir);
    ThreadPool pool(10);
    for(int i = 1; i <= block_number; i++){
        std::string file_name =dir + std::string("test.txt") + std::to_string(i);

        //Task task(file_name,"127.0.0.1",atoi(argv[1]));
        Task task;
        pool.addTask(std::bind(&Task::process,&task,file_name,"127.0.0.1",atoi(argv[1])));
        //task.process();
    }
    while(1){
        if(pool.size() == 0){
            pool.stop();
            printf("Now I will exit from main\n");
            break;
        }
        sleep(2);
    }

    return 0;
}

