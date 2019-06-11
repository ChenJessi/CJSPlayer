//
// Created by CHEN on 2019/3/18.
//


#include "CyVideo.h"

CyVideo::CyVideo(CyPlaystatus *playstatus, CyCallJava *callJava) {
    this->playstatus = playstatus;
    this->callJava = callJava;
    queue = new CyQueue(playstatus);
    pthread_mutex_init(&codecMutex, NULL);
}

CyVideo::~CyVideo() {
    pthread_mutex_destroy(&codecMutex);
}

void *playVideo(void *data) {
    CyVideo *video = static_cast<CyVideo *>(data);
    while (video->playstatus != NULL && !video->playstatus->exit) {
        if (video->playstatus->seek) {
            av_usleep(1000 * 100);
            continue;
        }
        if (video->playstatus->pause) {
            av_usleep(1000 * 100);
            continue;
        }
        if (video->queue->getQueueSize() == 0) {
            if (!video->playstatus->load) {
                video->playstatus->load = true;
                video->callJava->onCallLoad(CHILD_THREAD, true);
            }
            av_usleep(1000 * 100);
            continue;
        } else {
            if (video->playstatus->load) {
                video->playstatus->load = false;
                video->callJava->onCallLoad(CHILD_THREAD, false);
            }
        }
        AVPacket *avPacket = av_packet_alloc();
        if (video->queue->getAvpacket(avPacket) != 0) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        if (video->codectype == CODEC_MEDIACODEC) {
            //硬解码
            if (av_bsf_send_packet(video->abs_ctx, avPacket) != 0){
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                continue;
            }
            while (av_bsf_receive_packet(video->abs_ctx,avPacket) == 0){
                double  diff = video->getFrameDiffTime(NULL, avPacket);
                av_usleep(video->getDelayTime(diff) * 1000000);
                video->callJava->onCallDecodeAVPacket(avPacket->size, avPacket->data);

                av_packet_free(&avPacket);
                av_free(avPacket);
                continue;
            }
            avPacket == NULL;

        } else if (video->codectype == CODEC_YUV) {
            pthread_mutex_lock(&video->codecMutex);
            int ret = avcodec_send_packet(video->avCodecContext, avPacket);
            if (ret != 0) {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                pthread_mutex_unlock(&video->codecMutex);
                continue;
            }

            AVFrame *avFrame = av_frame_alloc();
            if (avcodec_receive_frame(video->avCodecContext, avFrame) != NULL) {
                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                pthread_mutex_unlock(&video->codecMutex);
                continue;
            }
            if (avFrame->format == AV_PIX_FMT_YUV420P) {
                LOGE("当前视频是YUV420P格式");
                double diff = video->getFrameDiffTime(avFrame ,NULL);
                av_usleep(video->getDelayTime(diff) * 1000000);
                video->callJava->onCallRenderYUV(
                        video->avCodecContext->width,
                        video->avCodecContext->height,
                        avFrame->data[0],
                        avFrame->data[1],
                        avFrame->data[2]);
            } else {
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
                if (!sws_ctx) {
                    av_frame_free(&pFrameYUV420P);
                    av_free(pFrameYUV420P);
                    pFrameYUV420P = NULL;
                    pthread_mutex_unlock(&video->codecMutex);
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
                double diff = video->getFrameDiffTime(avFrame, NULL);
                av_usleep(video->getDelayTime(diff) * 1000000);
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
            pthread_mutex_unlock(&video->codecMutex);
        }
    }
    return 0;
}

void CyVideo::play() {
    pthread_create(&pthread_play, NULL, playVideo, this);
}

void CyVideo::release() {
    if(queue != NULL)
    {
        queue->noticeQueue();
    }
    pthread_join(pthread_play, NULL);
    if (queue != NULL) {
        delete (queue);
        queue = NULL;
    }
    if (abs_ctx != NULL){
        av_bsf_free(&abs_ctx);
        abs_ctx = NULL;
    }
    if (avCodecContext != NULL) {
        pthread_mutex_lock(&codecMutex);
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = NULL;
        pthread_mutex_unlock(&codecMutex);
    }
    if (playstatus != NULL) {
        playstatus = NULL;
    }
    if (callJava != NULL) {
        callJava = NULL;
    }
}


double CyVideo::getFrameDiffTime(AVFrame *avFrame, AVPacket *avPacket) {
    double pts = 0;
    if (avFrame != NULL){
        pts = avFrame->pts;
    }
    if (avPacket != NULL){
        pts = avPacket->pts;
    }
    if (pts == AV_NOPTS_VALUE) {
        pts = 0;
    }
    pts *= av_q2d(time_base);
    if (pts > 0) {
        clock = pts;
    }
    double diff = audio->clock - clock;
    return diff;
}

double CyVideo::getDelayTime(double diff) {
    if (diff > 0.003) {
        delayTime = delayTime * 2 / 3;
        if (delayTime < defaultDelayTime / 2) {
            delayTime = defaultDelayTime * 2 / 3;
        } else if (delayTime > defaultDelayTime * 2) {
            delayTime = defaultDelayTime * 2;
        }
    } else if (diff < -0.003) {
        delayTime = delayTime * 3 / 2;
        if (delayTime < defaultDelayTime / 2) {
            delayTime = defaultDelayTime * 2 / 3;
        } else if (delayTime > defaultDelayTime * 2) {
            delayTime = defaultDelayTime * 2;
        }
    } else if (diff == 0.003) {

    }
    if (diff >= 0.5) {
        delayTime = 0;
    } else if (diff <= -0.5) {
        delayTime = defaultDelayTime * 2;
    }
    if (fabs(diff) >= 10) {
        delayTime = defaultDelayTime;
    }
    return delayTime;
}

