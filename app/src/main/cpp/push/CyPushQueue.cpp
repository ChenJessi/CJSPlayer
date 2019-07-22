//
// Created by CHEN on 2019/7/22.
//

#include "CyPushQueue.h"

CyPushQueue::CyPushQueue() {
    pthread_mutex_init(&mutexPacket, NULL);
    pthread_cond_init(&condPacket, NULL);
}

CyPushQueue::~CyPushQueue() {
    clearQueue();
    pthread_mutex_destroy(&mutexPacket);
    pthread_cond_destroy(&condPacket);
}

int CyPushQueue::putRtmpPacket(RTMPPacket *packet) {
    pthread_mutex_lock(&mutexPacket);
    queuePacket.push(packet);
    pthread_cond_signal(&condPacket);
    pthread_mutex_unlock(&mutexPacket);
    return 0;
}

RTMPPacket *CyPushQueue::getRtmpPacket() {
    pthread_mutex_lock(&mutexPacket);
    RTMPPacket *p = NULL;
    if (!queuePacket.empty()){
        p = queuePacket.front();
        queuePacket.pop();
    } else{
        pthread_cond_wait(&condPacket, &mutexPacket);
    }
    pthread_mutex_unlock(&mutexPacket);
    return p;
}

void CyPushQueue::clearQueue() {
    pthread_mutex_lock(&mutexPacket);
    while (true){
        if (queuePacket.empty()){
            break;
        }
        RTMPPacket *p = queuePacket.front();
        queuePacket.pop();
        RTMPPacket_Free(p);
    }
    pthread_mutex_unlock(&mutexPacket);
}

void CyPushQueue::notifyQueue() {
    pthread_mutex_lock(&mutexPacket);
    pthread_cond_signal(&condPacket);
    pthread_mutex_unlock(&mutexPacket);

}
