/*************************************************************************
	> File Name: safe_structure.h
	> Author: 
	> Mail: 
	> Created Time: 2017年05月13日 星期六 16时04分41秒
 ************************************************************************/

#ifndef _SAFE_STRUCTURE_H
#define _SAFE_STRUCTURE_H
#include<mutex>
#include<condition_variable>
#include<vector>
#include<queue>
#include<exception>
#include<stdexcept>


//线程安全的队列
//进行任务的添加
template<class T>
class safe_queue{
private:
    std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;
public:
    safe_queue():data_queue(){}

    bool empty(){
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }

    void push(T value){
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(value);
        data_cond.notify_one();
    }

    T front(){
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk,[this]{return !data_queue.empty();});
        return data_queue.front();
    }

    void pop(){
        if(empty())
            throw std::range_error("empty");
        std::lock_guard<std::mutex> lk(mut);
        data_queue.pop();
    }

    T wait_and_pop(){
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk,[this]{return !data_queue.empty();});
        T value = data_queue.front();
        data_queue.pop();
        return value;
    }

    T front_and_pop(){
        std::lock_guard<std::mutex> lk(mut);
        if(empty())
            throw std::range_error("empty");
        T value = data_queue.front();
        data_queue.pop();
        return value;
    }

};

#endif
