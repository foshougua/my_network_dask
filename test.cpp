/*************************************************************************
	> File Name: test.cpp
	> Author: 
	> Mail: 
	> Created Time: 2017年05月29日 星期一 09时36分15秒
 ************************************************************************/

#include<iostream>
#include<string>
#include<cstdio>
#include<cstring>
#include<map>
int main(){
    std::map<std::string,std::string> m;
    m["book1"] = std::string("wahaha");
    m["book2"] = std::string("liuliu");
    std::map<std::string,std::string>::iterator it;
    for(it = m.begin(); it != m.end(); it++){
        std::cout<<it->first<<std::endl<<it->second<<std::endl;
    }
}
