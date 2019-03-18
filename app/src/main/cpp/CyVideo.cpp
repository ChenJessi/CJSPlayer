//
// Created by CHEN on 2019/3/18.
//

#include "CyVideo.h"

CyVideo::CyVideo(CyPlaystatus *playstatus, CyCallJava *callJava) {
    this->playstatus = playstatus;
    this->callJava = callJava;
    queue = new CyQueue(playstatus);
}

CyVideo::~CyVideo() {

}
void * playVideo(void *data){
    CyVideo *video = static_cast<CyVideo *>(data);
    while (video->playstatus != NULL && !video->playstatus->exit){
        AVPacket *avPacket = av_packet_alloc();
        if (video->queue->getAvpacket(avPacket) == 0){
            //解码渲染
            LOGE("获取视频avpacket")
        }
        av_packet_free(&avPacket);
        av_free(avPacket);
        avPacket = NULL;
    }
    pthread_exit(&video->pthread_play);
}

void CyVideo::play() {
    pthread_create(&pthread_play, NULL, playVideo, this);
}
