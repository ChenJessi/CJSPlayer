//
// Created by CHEN on 2019/7/22.
//

#include "RtmpPush.h"


RtmpPush::RtmpPush(const char *url, CyCallJava *callJava) {
    this->url = static_cast<char *>(malloc(512));
    strcpy(this->url, url);
    this->queue = new CyPushQueue();
    this->callJava = callJava;
}

RtmpPush::~RtmpPush() {
    queue->notifyQueue();
    queue->clearQueue();
    free(url);
}

void *callBackPush(void *data){
    RtmpPush *rtmpPush = static_cast<RtmpPush *>(data);

    rtmpPush->rtmp = RTMP_Alloc();
    RTMP_Init(rtmpPush->rtmp);
    rtmpPush->rtmp->Link.timeout = 10;
    rtmpPush->rtmp->Link.lFlags |= RTMP_LF_LIVE;
    RTMP_SetupURL(rtmpPush->rtmp, rtmpPush->url);
    RTMP_EnableWrite(rtmpPush->rtmp);

    if (!RTMP_Connect(rtmpPush->rtmp, NULL)){
        LOGE("can not connect the url");
        rtmpPush->callJava->onConnectFail("can not connect the url");
        goto end;
    }
    if(!RTMP_ConnectStream(rtmpPush->rtmp, 0)) {
        LOGE("can not connect the stream of service");
        rtmpPush->callJava->onConnectFail("can not connect the stream of service");
        goto end;
    }
    LOGD("链接成功， 开始推流");
    rtmpPush->callJava->onConnectsuccess();

    end:
    RTMP_Close(rtmpPush->rtmp);
    RTMP_Free(rtmpPush->rtmp);
    rtmpPush->rtmp = NULL;
    pthread_exit(&rtmpPush->push_thread);
}

void RtmpPush::init() {
    callJava->onConnectint(CHILD_THREAD);
    pthread_create(&push_thread, NULL, callBackPush, this);
}
