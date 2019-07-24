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
#include "CyCallJava.h"
#include "../CyCallJava.h"

extern "C"{
#include "../librtmp/rtmp.h"
};
class RtmpPush {
public:
    RTMP *rtmp = NULL;
    char *url = NULL;
    CyPushQueue *queue = NULL;
    pthread_t push_thread;
    CyCallJava *callJava = NULL;

public:
    RtmpPush(const char *url, CyCallJava *callJava);
    ~RtmpPush();

    void init();
};


#endif //CYPLAYER_RTMPPUSH_H
