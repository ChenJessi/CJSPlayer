//
// Created by 哦 on 2023/2/20.
//

#ifndef CJSPLAYER_SAFE_QUEUE_H
#define CJSPLAYER_SAFE_QUEUE_H

#include <queue>
#include <pthread.h>

using namespace std;

template<typename T>
class SafeQueue {

private:
    typedef void(*ReleaseCallback)(T*);
    typedef void(*SyncCallback)(queue<T>&);
private:
    queue<T> queue_data;
    // 互斥锁
    pthread_mutex_t mutex;
    // 唤醒，等待
    pthread_cond_t cond;
    //标记队列是否处于工作中
    int isWork = 0;

    ReleaseCallback releaseCallback = nullptr;
    SyncCallback syncCallback = nullptr;
public:
    SafeQueue(){
        pthread_cond_init(&cond, 0);
        pthread_mutex_init(&mutex,0);
    };
    ~SafeQueue(){
        LOGE("SafeQueue 析构函数 %d", queue_data.size());
        pthread_cond_destroy(&cond);
        pthread_mutex_destroy(&mutex);
    };
    /**
     * 入队数据包 [AVPacket* 压缩包][AVFrame * 原始包]
     * @param value
     */
    void insertToQueue(T value){
        pthread_mutex_lock(&mutex);
        if(isWork){
            queue_data.push(value);
            // 唤醒工作线程
            pthread_cond_signal(&cond);
        }
        else {
            // 非工作状态，释放
            if (releaseCallback){
                releaseCallback(&value);
            }
        }

        pthread_mutex_unlock(&mutex);
    }

    /**
     *  出队数据包 [AVPacket* 压缩包]  [AVFrame* 原始包]
     * @param value
     */
    int getQueueAndDel(T &value){
        int ret = 0;
        pthread_mutex_lock(&mutex);

        while (isWork && queue_data.empty()){
            // 线程工作中 并且 队列没有数据的时候，阻塞线程等待
            pthread_cond_wait(&cond, &mutex);
        }
        //
        if(!queue_data.empty()){
            // 去出数据包并删除队列里的数据
            value = queue_data.front();
            queue_data.pop();
            ret = 1;
        }
        pthread_mutex_unlock(&mutex);

        return ret;
    };

    /**
     * 设置队列是否工作
     */
    void setWork(int work){
        pthread_mutex_lock(&mutex);

        this->isWork = work;
        // 工作状态变更之后，唤醒一下线程
        pthread_cond_signal(&cond);

        pthread_mutex_unlock(&mutex);
    };

    int empty(){
        return queue_data.empty();
    }

    int size(){
        return queue_data.size();
    }

    // 清空队列
    void clear(){
        pthread_mutex_lock(&mutex);

        while (!queue_data.empty()){
            if(releaseCallback){
                releaseCallback(&queue_data.front());
            }
            queue_data.pop();
        }

        pthread_mutex_unlock(&mutex);
    }

    /**
     *  函数指针回调，用于释放资源
     */
    void setReleaseCallback(ReleaseCallback callback){
        this->releaseCallback = callback;
    }

    /**
     * 音视频同步逻辑的回调
     * @param callback
     */
    void setSyncCallback(SyncCallback callback){
        this->syncCallback = callback;
    }

    /**
     * 处理音视频同步的问题
     */
    void sync(){
        pthread_mutex_lock(&mutex);
        syncCallback(queue_data);
        pthread_mutex_unlock(&mutex);
    }

};



#endif //CJSPLAYER_SAFE_QUEUE_H
