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

#include "../push/CyPushCallJava.h"

extern "C"{
#include "../librtmp/rtmp.h"
};
class RtmpPush {
public:
    RTMP *rtmp = NULL;
    char *url = NULL;
    CyPushQueue *queue = NULL;
    pthread_t push_thread;
    CyPushCallJava *callJava = NULL;

    bool startPushing = false;
    long startTime = 0;
public:
    RtmpPush(const char *url, CyPushCallJava *callJava);
    ~RtmpPush();

    void init();

    void pushSPSPPS(char *sps, int sps_len, char *pps, int pps_len);

    void pushVideoData(char *data, int data_len, bool keyframe);
};


#endif //CYPLAYER_RTMPPUSH_H
