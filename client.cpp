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
#include"handle_request.h"
#include"upload_file.h"
#include"download_file.h"
#include"time_machine.h"

int main(int argc,char **argv){
    if(argc < 2){
        std::cerr<<"need input server port!";
        exit(1);
    }

    /*OperateFile temp;
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
    }*/

    Client cli;
    cli.create_socket();
    cli.connect_to_server("127.0.0.1",atoi(argv[1]));
    int fd = cli.get_fd();
    /*UploadFile upload("12345","test.txt");
    upload.SendRequest(fd);
    std::vector<std::string> md5_block_vector = upload.ReciveBlockList(fd);
    upload.SendBlockFile(fd,md5_block_vector,"127.0.0.1",atoi(argv[1]));*/
    TimeMachine time("12345","test.txt",1);
    time.SendRequest(fd);
    //char buff[1024];
    //memset(buff,'a',1024);
    //send(fd,buff,1024,0);
    return 0;
}

