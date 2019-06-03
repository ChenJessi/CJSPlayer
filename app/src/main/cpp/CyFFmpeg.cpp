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
void CyFFmpeg::prepared() {
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

    //获取音视频流
    for (int i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (audio == NULL) {
                audio = new CyAudio(playstatus, pFormatCtx->streams[i]->codecpar->sample_rate,callJava);
                audio->streamIndex = i;
                audio->codecpar = pFormatCtx->streams[i]->codecpar;
                audio->duration = pFormatCtx->duration / AV_TIME_BASE;
                audio->time_base = pFormatCtx->streams[i]->time_base;
                duration = audio->duration;
                callJava->onCallPcmRate(audio->sample_rate);
            }
        } else if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            if (video == NULL){
                video = new CyVideo(playstatus, callJava);
                video->streamIndex = i;
                video->codecpar = pFormatCtx->streams[i]->codecpar;
                video->time_base = pFormatCtx->streams[i]->time_base;
            }
        }
    }
    if (audio != NULL){
        getCodecContext(audio->codecpar , &audio->avCodecContext);
    }
    if (video != NULL){
        getCodecContext(video->codecpar , &video->avCodecContext);
    }
    if (callJava != NULL){
        if (playstatus != NULL && !playstatus->exit){
            callJava->onCallPrepared(CHILD_THREAD);
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
    if (video == NULL) {
        return;
    }
    supportMediacodec = false;
    video->audio = audio;
    const  char* codename = video->avCodecContext->codec->name;

    if (supportMediacodec = callJava->onCallSupportVideo(codename)){
        LOGE("当前设备支持硬解码当前视频")
        //找到相应解码器的过滤器
        if (strcasecmp(codename,"h264") == 0){
            bsFilter = av_bsf_get_by_name("h264_mp4toannexb");
        } else if (strcasecmp(codename,"h265")){
            bsFilter = av_bsf_get_by_name("hevc_mp4toannexb");
        }
        if (bsFilter == NULL){
            goto end;
        }
        if (av_bsf_alloc(bsFilter,&video->abs_ctx) != 0){
            supportMediacodec = false;
            goto end;
        }
        //添加解码器属性
        if (avcodec_parameters_copy(video->abs_ctx->par_in,video->codecpar) < 0 ){
            supportMediacodec = false;
            av_bsf_free(&video->abs_ctx);
            video->abs_ctx = NULL;
            goto end;
        }
        if (av_bsf_init(video->abs_ctx) != 0){
            supportMediacodec = false;
            av_bsf_free(&video->abs_ctx);
            video->abs_ctx = NULL;
            goto end;
        }
        video->abs_ctx->time_base_in = video->time_base;
    }
    end:
    if (supportMediacodec){
        video->codectype = CODEC_MEDIACODEC;
    }
    audio->play();
    video->play();
    while (playstatus != NULL && !playstatus->exit) {
        if(playstatus->seek) {
            av_usleep(1000 * 100);
            continue;
        }
        if(audio->queue->getQueueSize() > 40) {
            av_usleep(1000 * 100);
            continue;
        }
        //读取音频帧
        AVPacket *avPacket = av_packet_alloc();
        if(av_read_frame(pFormatCtx, avPacket) == 0) {
            if(avPacket->stream_index == audio->streamIndex) {
                audio->queue->putAvpacket(avPacket);
            } else if(avPacket->stream_index == video->streamIndex) {
                video->queue->putAvpacket(avPacket);
            } else{
                av_packet_free(&avPacket);
                av_free(avPacket);
            }
        } else{
            av_packet_free(&avPacket);
            av_free(avPacket);
            while(playstatus != NULL && !playstatus->exit) {
                if(audio->queue->getQueueSize() > 0) {
                    av_usleep(1000 * 100);
                    continue;
                } else{
                    if(!playstatus->seek) {
                        av_usleep(1000 * 100);
                        playstatus->exit = true;
                    }
                    break;
                }
            }
            break;
        }
    }
    if (callJava != NULL){
        callJava->onCallComplete(CHILD_THREAD);
    }
    exit = true;
}

void CyFFmpeg::pause() {
    if (playstatus != NULL){
        playstatus->pause = true;
    }
    if (audio != NULL){
        audio->pause();
    }
}

void CyFFmpeg::resume() {
    if (playstatus != NULL){
        playstatus->pause = false;
    }
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
        LOGD("start release ffmpeg")
    }
    playstatus->exit = true;
    pthread_mutex_lock(&init_mutex);
    int sleepCount = 0;
    while (!exit){
        if (sleepCount > 1000){
            exit = true;
        }
        if (LOG_DEBUG){
            LOGD("wait ffmpeg exit");
        }
        sleepCount++;
        av_usleep(1000 * 10);
    }

    if (LOG_DEBUG){
        LOGD("release audio;")
    }

    if (audio != NULL){
        audio->clock = 0;
        audio->last_time = 0;
        audio->isCut = false;
        audio->end_time = 0;
        audio->showPcm = false;
        audio->release();
        delete(audio);
        audio = NULL;
    }
    if(LOG_DEBUG) {
        LOGD("释放 video");
    }
    if(video != NULL) {
        video->release();
        delete(video);
        video = NULL;
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

int CyFFmpeg::getCodecContext(AVCodecParameters *codecpar, AVCodecContext **avCodecContext) {
    //获取解码器
    AVCodec *dec = avcodec_find_decoder(codecpar->codec_id);
    if (!dec) {
        if (LOG_DEBUG) {
            LOGE("can not find decoder!")
        }
        callJava->onCallError(CHILD_THREAD, 1003, "can not find decoder");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }

    //创建解码器上下文
    *avCodecContext = avcodec_alloc_context3(dec);
    if (!*avCodecContext) {
        if (LOG_DEBUG) {
            LOGE("can not alloc new decodecctx")
        }
        callJava->onCallError(CHILD_THREAD, 1004, "can not alloc new decodecctx");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }
    //将codecpar复制到解码器上下文
    if (avcodec_parameters_to_context(*avCodecContext, codecpar) < 0) {
        if (LOG_DEBUG) {
            LOGE("can not fill decodecctx!");
        }
        callJava->onCallError(CHILD_THREAD, 1005, "an not fill decodecctx");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }
    //打开解码器
    if (avcodec_open2(*avCodecContext, dec, 0) != 0) {
        if (LOG_DEBUG) {
            LOGE("can not open audio strames!")
        }
        callJava->onCallError(CHILD_THREAD, 1006, "can not open audio !");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }
    return 0;
}


void CyFFmpeg::seek(int64_t secds) {
    if (duration <= 0){
        return;
    }
    if (secds >= 0 && secds <= duration){
        playstatus->seek =  true;
        pthread_mutex_lock(&seek_mutex);
        int64_t  rel = secds * AV_TIME_BASE;
        avformat_seek_file(pFormatCtx, -1 , INT64_MIN, rel, INT64_MAX, 0);
        if (audio != NULL){
            audio->queue->clearAvpacket();
            audio->clock = 0;
            audio->last_time = 0;
            pthread_mutex_lock(&audio->codecMutex);
            avcodec_flush_buffers(audio->avCodecContext);
            pthread_mutex_unlock(&audio->codecMutex);
        }
        if (video != NULL){
            video->queue->clearAvpacket();
            video->clock = 0;
            pthread_mutex_lock(&video->codecMutex);
            avcodec_flush_buffers(video->avCodecContext);
            pthread_mutex_unlock(&video->codecMutex);
        }
        pthread_mutex_unlock(&seek_mutex);
        playstatus->seek = false;
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

int CyFFmpeg::getSampleRate() {
    if (audio != NULL){
        return  audio->avCodecContext->sample_rate;
    }
    return  0;
}

void CyFFmpeg::startStopRecord(bool start) {
    if (audio != NULL){
        audio->startStopRecord(start);
    }
}

void CyFFmpeg::stop() {
    if (audio != NULL){
        audio->stop();
    }
}

bool CyFFmpeg::cutAudioPlay(int start_time, int end_time, bool showPcm) {
    if (start_time >= 0 && end_time <= duration && start_time < end_time){
        audio->isCut = true;
        audio->end_time = end_time;
        audio->showPcm = showPcm;
        seek(start_time);
        return true;
    }
    return false;
}



