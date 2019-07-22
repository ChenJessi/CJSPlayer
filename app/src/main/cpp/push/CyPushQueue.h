//
// Created by CHEN on 2019/7/22.
//

#ifndef CYPLAYER_CYPUSHQUEUE_H
#define CYPLAYER_CYPUSHQUEUE_H

#include "queue"
#include "pthread.h"
#include "../AndroidLog.h"

extern "C"{
#include "../librtmp/rtmp.h"
};
class CyPushQueue {
public:
    std::queue<RTMPPacket *> queuePacket;
    pthread_mutex_t mutexPacket;
    pthread_cond_t condPacket;

public:
    CyPushQueue();
    ~CyPushQueue();

    int putRtmpPacket(RTMPPacket *packet);

    RTMPPacket* getRtmpPacket();

    void clearQueue();

    void notifyQueue();
};


#endif //CYPLAYER_CYPUSHQUEUE_H
