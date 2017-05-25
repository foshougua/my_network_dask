/*************************************************************************
	> File Name: test.cpp
	> Author: 
	> Mail: 
	> Created Time: 2017年05月13日 星期六 09时49分21秒
 ************************************************************************/

#include<iostream>
#include<cstring>
#include<fstream>
#include"task.h"
#include"file.h"
int main(){
    std::ifstream in;
    in.open("test.txt");
    in.seekg(0,std::ios::beg);
    std::cout<<"location = "<<in.tellg()<<std::endl;
}
