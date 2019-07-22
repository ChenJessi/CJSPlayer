//
// Created by CHEN on 2019/7/22.
//

#ifndef CYPLAYER_RTMPPUSH_H
#define CYPLAYER_RTMPPUSH_H
#include "../AndroidLog.h"
#include <malloc.h>
#include <string.h>
#include "pthread.h"
#include "CyPushQueue.h"

extern "C"{
#include "../librtmp/rtmp.h"
};
class RtmpPush {
public:
    RTMP *rtmp = NULL;
    char *url = NULL;
    CyPushQueue *queue = NULL;
    pthread_t push_thread;

public:
    RtmpPush(const char *url);
    ~RtmpPush();

    void init();
};


#endif //CYPLAYER_RTMPPUSH_H
