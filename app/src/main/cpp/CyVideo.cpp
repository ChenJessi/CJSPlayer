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
        if (video->playstatus->seek){
            av_usleep(1000 * 100);
            continue;
        }
        if (video->queue->getQueueSize() == 0){
            if (!video->playstatus->load){
                video->playstatus->load = true;
                video->callJava->onCallLoad(CHILD_THREAD, true);
            }
            av_usleep(1000 * 100);
            continue;
        } else{
            if (video->playstatus->load){
                video->playstatus->load = false;
                video->callJava->onCallLoad(CHILD_THREAD, false);
            }
        }
        AVPacket *avPacket = av_packet_alloc();
        if (video->queue->getAvpacket(avPacket) != 0){
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        int ret = avcodec_send_packet(video->avCodecContext, avPacket);
        LOGD("ret  : %d", ret)
        if ( ret != 0){
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }

        AVFrame *avFrame = av_frame_alloc();
        if (avcodec_receive_frame(video->avCodecContext, avFrame) != NULL){
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        LOGD("视频解码帧")
        av_frame_free(&avFrame);
        av_free(avFrame);
        avFrame = NULL;
        av_packet_free(&avPacket);
        av_free(avPacket);
        avPacket = NULL;
    }
    pthread_exit(&video->pthread_play);
}

void CyVideo::play() {
    pthread_create(&pthread_play, NULL, playVideo, this);
}

void CyVideo::release() {
    if (queue != NULL){
        delete(queue);
        queue = NULL;
    }
    if(avCodecContext != NULL) {
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = NULL;
    }
    if(playstatus != NULL) {
        playstatus = NULL;
    }
    if(callJava != NULL) {
        callJava = NULL;
    }
}
