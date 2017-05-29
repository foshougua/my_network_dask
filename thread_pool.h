/*************************************************************************
	> File Name: thread_pool.h
	> Author: 
	> Mail: 
	> Created Time: 2017年05月13日 星期六 12时22分24秒
 ************************************************************************/

#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <deque>  
#include <pthread.h>
#include <functional>// for std::function, std::bind

class ThreadPool{
public:
    typedef std::function<void()> Task;
    ThreadPool(int threadNum = 10);
    ~ThreadPool();
    size_t addTask(const Task& task);
    void   stop();
    int    size();
    Task take();

private:
    int createThreads();
    static void* threadFunc(void * threadData);//这里threadFun是静态的
    ThreadPool& operator=(const ThreadPool&);
    ThreadPool(const ThreadPool&);

private:
    volatile bool isRunning_;
    int threadsNum_;
    pthread_t* threads_;
    std::deque<Task> taskQueue_;
    pthread_mutex_t mutex_;
    pthread_cond_t condition_;
};

//构造函数，创建一定数量的线程，要调用create函数
ThreadPool::ThreadPool(int threadNum){
    isRunning_ = true;
    threadsNum_ = threadNum;
    createThreads();
}

ThreadPool::~ThreadPool(){
    stop();
}

//创建线程
int ThreadPool::createThreads(){
    pthread_mutex_init(&mutex_, NULL);
    pthread_cond_init(&condition_, NULL);
    threads_ = (pthread_t*) malloc(sizeof(pthread_t) * threadsNum_);
    for (int i = 0; i < threadsNum_; i++){
        pthread_create(&threads_[i], NULL, threadFunc, this);
    }
    return 0;
}

//往线程队列里添加线程
size_t ThreadPool::addTask(const Task& task){
    pthread_mutex_lock(&mutex_);
    taskQueue_.push_back(task);
    int size = taskQueue_.size();
    pthread_mutex_unlock(&mutex_);
    pthread_cond_signal(&condition_);
    return size;
}

//停止所有的线程
//释放资源
void ThreadPool::stop(){
    if (!isRunning_){
        return;
    }

    isRunning_ = false;
    pthread_cond_broadcast(&condition_);

    for (int i = 0; i < threadsNum_; i++){
        pthread_join(threads_[i], NULL);//等待其他线程退出
    }

    free(threads_);
    threads_ = NULL;
    pthread_mutex_destroy(&mutex_);
    pthread_cond_destroy(&condition_);
}

//任务队列的大小
int ThreadPool::size(){
    pthread_mutex_lock(&mutex_);
    int size = taskQueue_.size();
    pthread_mutex_unlock(&mutex_);
    return size;
}

//从任务队列中拿出一个任务
ThreadPool::Task ThreadPool::take(){
    Task task = NULL;
    pthread_mutex_lock(&mutex_);
    while (taskQueue_.empty() && isRunning_){
        pthread_cond_wait(&condition_, &mutex_);
    }
    if (!isRunning_){
        pthread_mutex_unlock(&mutex_);  
        return task;
    }
    assert(!taskQueue_.empty());
    task = taskQueue_.front();
    taskQueue_.pop_front();
    pthread_mutex_unlock(&mutex_);
    return task;
}

//创建线程时的驱动函数，如果有task就运行它
void* ThreadPool::threadFunc(void* arg){
    pthread_t tid = pthread_self();
    ThreadPool* pool = static_cast<ThreadPool*>(arg);
    while (pool->isRunning_){
        ThreadPool::Task task = pool->take();
        if (!task){
            printf("thread %lu will exit\n", tid);
            break;
        }
        assert(task);
        task();
    }
    return 0;
}

#endif
