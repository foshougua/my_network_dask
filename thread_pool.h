/*************************************************************************
	> File Name: thread_pool.h
	> Author: 
	> Mail: 
	> Created Time: 2017年05月13日 星期六 12时22分24秒
 ************************************************************************/

#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H
#include<thread>
#include<queue>
#include<vector>
#include<atomic>
#include<mutex>
#include<condition_variable>


class join_threads{
public:
    explicit join_threads(std::vector<std::thread>& t):threads(t){}

    ~join_threads(){
        for(unsigned long i = 0; i < threads.size(); i++){
            if(threads[i].joinable()){
                threads[i].join();
            }
        }
    }

private:
    std::vector<std::thread>& threads;
};

template<class T>
class thread_pool{
public:
    thread_pool(unsigned int count):thread_count(count),done(false),joiner(threads){}

    void start(){
        try{
            for(int i = 0; i< thread_count; i++){
                threads.push_back(std::thread(&thread_pool::worker_thread,this));
            }
        }
        catch(...){//创建线程可能失败
            done = true;
            throw;
        }
    }

    void add_task(T f){
        std::lock_guard<std::mutex> lk(mut);
        work_queue.push(f);
    }

    ~thread_pool(){
        done = true;
    }

private:

    bool work_queue_empty(){
        std::lock_guard<std::mutex> lk(mut);
        return work_queue.empty();
    }

    void worker_thread(){
        while(!done){
            if(!work_queue_empty()){
                std::lock_guard<std::mutex> lk(mut);
                T task = work_queue.front();
                work_queue.pop();
                task->process();
            }
        }
    }

private:
    std::mutex mut;
    std::condition_variable data_cond;
    unsigned int thread_count;
    std::atomic_bool done;//代表任务是否完成
    std::queue<T> work_queue;//工作队列
    std::vector<std::thread> threads;//放置线程的vector
    join_threads joiner;//所有的线程调用join，等待它们完成
};

#endif
