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
        if (avFrame->format == AV_PIX_FMT_YUV420P){
            LOGE("当前视频是YUV420P格式");
            video->callJava->onCallRenderYUV(
                    video->avCodecContext->width,
                    video->avCodecContext->height,
                    avFrame->data[0],
                    avFrame->data[1],
                    avFrame->data[2]);
        } else{
            LOGE("当前视频不是YUV420P格式");
            AVFrame *pFrameYUV420P = av_frame_alloc();
            int num = av_image_get_buffer_size(
                    AV_PIX_FMT_YUV420P,
                    video->avCodecContext->width,
                    video->avCodecContext->height,
                    1);
            uint8_t *buffer = static_cast<uint8_t *>(av_malloc(num * sizeof(uint8_t)));
            av_image_fill_arrays(
                    pFrameYUV420P->data,
                    pFrameYUV420P->linesize,
                    buffer,
                    AV_PIX_FMT_YUV420P,
                    video->avCodecContext->width,
                    video->avCodecContext->height,
                    1);
            SwsContext *sws_ctx = sws_getContext(
                    video->avCodecContext->width,
                    video->avCodecContext->height,
                    video->avCodecContext->pix_fmt,
                    video->avCodecContext->width,
                    video->avCodecContext->height,
                    AV_PIX_FMT_YUV420P,
                    SWS_BICUBIC, NULL, NULL, NULL);
            if (!sws_ctx){
                av_frame_free(&pFrameYUV420P);
                av_free(pFrameYUV420P);
                pFrameYUV420P = NULL;
                continue;
            }
            sws_scale(
                    sws_ctx,
                    avFrame->data,
                    avFrame->linesize,
                    0,
                    avFrame->height,
                    pFrameYUV420P->data,
                    pFrameYUV420P->linesize);
            //渲染
            video->callJava->onCallRenderYUV(
                    video->avCodecContext->width,
                    video->avCodecContext->height,
                    pFrameYUV420P->data[0],
                    pFrameYUV420P->data[1],
                    pFrameYUV420P->data[2]);
            av_frame_free(&pFrameYUV420P);
            av_free(pFrameYUV420P);
            av_free(buffer);
            sws_freeContext(sws_ctx);
        }
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
