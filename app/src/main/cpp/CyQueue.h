//
// Created by CHEN on 2019/3/1.
//

#ifndef CYPLAYER_CYQUEUE_H
#define CYPLAYER_CYQUEUE_H

#include "queue"
#include "pthread.h"
#include "AndroidLog.h"
#include "CyPlaystatus.h"

extern "C"{
#include "libavcodec/avcodec.h"
};

class CyQueue {
    std::queue<AVPacket *> queuePacket;
    pthread_mutex_t mutexPacket;
    pthread_cond_t condPacket;
    CyPlaystatus *playstatus = NULL;

public:
    CyQueue(CyPlaystatus *playstatus);
    ~CyQueue();

    int putAvpacket(AVPacket *packet);

    int getAvpacket(AVPacket *packet);

    int getQueueSize();

    void clearAvpacket();

    void noticeQueue();
};


#endif //CYPLAYER_CYQUEUE_H
