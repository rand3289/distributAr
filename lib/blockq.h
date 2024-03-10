// Use this blocking queue to pass TimeBuffer from one thread to another
// When cluster thread has some nodes to compute, use popNoBlock() to check if there are new TimeBufferS
// when cluster thread does NOT have any more nodes to compute, use pop()

#pragma once
#include <mutex>
#include <condition_variable>
#include <queue>


template <typename T>
class BlockingQueue{
    std::mutex mut;
    std::condition_variable cv;
    std::queue<T> q;
public:
    void push(const T& val);
    void pop(T& val);
    bool popNoBlock(T& val);
};


template <typename T>
void BlockingQueue<T>::push(const T& val){
    std::unique_lock<std::mutex> lock(mut);
    q.push(val);
    cv.notify_one();
}


template <typename T>
void BlockingQueue<T>::pop(T& val){
    std::unique_lock<std::mutex> lock(mut);
    while(q.empty()){ cv.wait(lock); }
    val = q.front();
    q.pop();
}


template <typename T>
bool BlockingQueue<T>::popNoBlock(T& val){
    std::unique_lock<std::mutex> lock(mut);
    if (q.empty()){ return false; }
    val = q.front();
    q.pop();
    return true;
}
