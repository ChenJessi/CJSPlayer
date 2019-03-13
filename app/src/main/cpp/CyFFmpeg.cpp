//
// Created by CHEN on 2019/2/26.
//

#include "CyFFmpeg.h"

CyFFmpeg::CyFFmpeg( CyPlaystatus *playstatus, CyCallJava *callJava, const char *url) {
    this->callJava = callJava;
    this->url = url;
    this->playstatus = playstatus;
    pthread_mutex_init(&init_mutex, NULL);
    pthread_mutex_init(&seek_mutex, NULL);
}

void *decodeFFmpeg(void *data) {
    CyFFmpeg *cyFFmpeg = (CyFFmpeg *) data;
    cyFFmpeg->decodeFFmpegThread();
    pthread_exit(&cyFFmpeg->decodeThread);
}

/**
 * 准备
 */
void CyFFmpeg::parpared() {
    pthread_create(&decodeThread, NULL, decodeFFmpeg, this);
}

int avformat_callback(void *ctx){
   CyFFmpeg *cyFFmpeg = (CyFFmpeg *)ctx;
   if (cyFFmpeg->playstatus->exit){
       return AVERROR_EOF;
   }
    return 0;
};

/**
 * 解码
 */
void CyFFmpeg::decodeFFmpegThread() {
    //打开文件
    pthread_mutex_lock(&init_mutex);
    avformat_network_init();
    pFormatCtx = avformat_alloc_context();

    pFormatCtx->interrupt_callback.callback = avformat_callback;
    pFormatCtx->interrupt_callback.opaque = this;
    if (avformat_open_input(&pFormatCtx, url, NULL, NULL) != 0) {
        if (LOG_DEBUG) {
            LOGE("can not open url :%s", url);
        }
        callJava->onCallError(CHILD_THREAD, 1001, "can not open url" );
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }
    //获取流信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        if (LOG_DEBUG) {
            LOGE("can not find streams from %s", url);
        }
        callJava->onCallError(CHILD_THREAD, 1002, "can not find audio from url");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }
    //获取音频流
    for (int i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (audio == NULL) {
                audio = new CyAudio(playstatus, pFormatCtx->streams[i]->codecpar->sample_rate,callJava);
                audio->streamIndex = i;
                audio->codecpar = pFormatCtx->streams[i]->codecpar;
                audio->duration = pFormatCtx->duration / AV_TIME_BASE;
                audio->time_base = pFormatCtx->streams[i]->time_base;
                duration = audio->duration;
            }
        }
    }
    //获取解码器
    AVCodec *dec = avcodec_find_decoder(audio->codecpar->codec_id);
    if (!dec) {
        if (LOG_DEBUG) {
            LOGE("can not find decoder!")
        }
        callJava->onCallError(CHILD_THREAD, 1003, "can not find decoder");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    //创建解码器上下文
    audio->avCodecContext = avcodec_alloc_context3(dec);
    if (!audio->avCodecContext) {
        if (LOG_DEBUG) {
            LOGE("can not alloc new decodecctx")
        }
        callJava->onCallError(CHILD_THREAD, 1004, "can not alloc new decodecctx");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }
    //将codecpar复制到解码器上下文
    if (avcodec_parameters_to_context(audio->avCodecContext, audio->codecpar) < 0) {
        if (LOG_DEBUG) {
            LOGE("can not fill decodecctx!");
        }
        callJava->onCallError(CHILD_THREAD, 1005, "an not fill decodecctx");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }
    //打开解码器
    if (avcodec_open2(audio->avCodecContext, dec, 0) != 0) {
        if (LOG_DEBUG) {
            LOGE("can not open audio strames!")
        }
        callJava->onCallError(CHILD_THREAD, 1006, "can not open audio !");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }
    if (callJava != NULL){
        if (playstatus != NULL && !playstatus->exit){
            callJava->onCallParpared(CHILD_THREAD);
        } else{
            exit = true;
        }
    }
    pthread_mutex_unlock(&init_mutex);
}

/**
 * 开始
 */
void CyFFmpeg::start() {
    if (audio == NULL) {
        return;
    }
    audio->play();

    while (playstatus != NULL && !playstatus->exit) {
        if (playstatus->seek){
            continue;
        }
        if (audio->queue->getQueueSize() > 40){
            continue;
        }
        //读取音频帧
        AVPacket *avPacket = av_packet_alloc();
        if (av_read_frame(pFormatCtx, avPacket) == 0) {
            if (avPacket->stream_index == audio->streamIndex) {

                audio->queue->putAvpacket(avPacket);
            } else{
                av_packet_free(&avPacket);
                av_free(avPacket);
            }
        } else {
            av_packet_free(&avPacket);
            av_free(avPacket);
            while (playstatus != NULL && !playstatus->exit){
                if (audio->queue->getQueueSize() > 0){
                    continue;
                } else{
                    playstatus->exit = true;
                    break;
                }
            }
        }
    }
    if (callJava != NULL){
        callJava->onCallComplete(CHILD_THREAD);
    }
    exit = true;
}

void CyFFmpeg::pause() {
    if (audio != NULL){
        audio->pause();
    }
}

void CyFFmpeg::resume() {
    if (audio != NULL){
        audio->resume();
    }
}

CyFFmpeg::~CyFFmpeg() {
    pthread_mutex_destroy(&init_mutex);
    pthread_mutex_destroy(&seek_mutex);
}

void CyFFmpeg::release() {
    if (LOG_DEBUG){
        LOGE("start release ffmpeg")
    }
    playstatus->exit = true;
    pthread_mutex_lock(&init_mutex);
    int sleepCount = 0;
    while (!exit){
        if (sleepCount > 1000){
            exit = true;
        }
        if (LOG_DEBUG){
            LOGD("wait ffmpeg exit %d",  sleepCount);
        }
        sleepCount++;
        av_usleep(1000 * 10);
    }

    if (LOG_DEBUG){
        LOGE("release audio;")
    }

    if (audio != NULL){
        audio->release();
        delete(audio);
        audio = NULL;
    }
   if (pFormatCtx != NULL){
       avformat_close_input(&pFormatCtx);
       avformat_free_context(pFormatCtx);
       pFormatCtx = NULL;
   }

    if(callJava != NULL) {
        callJava = NULL;
    }

    if(playstatus != NULL) {
        playstatus = NULL;
    }
    pthread_mutex_unlock(&init_mutex);
}

void CyFFmpeg::seek(int64_t secds) {
    if (duration <= 0){
        return;
    }
    if (secds >= 0 && secds <= duration){
        if (audio != NULL){
            playstatus->seek =  true;
            audio->queue->clearAvpacket();
            audio->clock = 0;
            audio->last_time = 0;
            pthread_mutex_lock(&seek_mutex);
            int64_t  rel = secds * AV_TIME_BASE;
            avformat_seek_file(pFormatCtx, -1 , INT64_MIN, rel, INT64_MAX, 0);
            pthread_mutex_unlock(&seek_mutex);
            playstatus->seek = false;
        }
    }
}

void CyFFmpeg::setVolume(int percent) {
    if (audio != NULL){
        audio->setVolume(percent);
    }
}

void CyFFmpeg::setMute(int mute) {
    if (audio != NULL){
        audio->setMute(mute);
    }

}

void CyFFmpeg::setPitch(float pitch) {
    if (audio != NULL){
        audio->setPitch(pitch);
    }
}

void CyFFmpeg::setSpeed(float speed) {
    if (audio != NULL){
        audio->setSpeed(speed);
    }
}
