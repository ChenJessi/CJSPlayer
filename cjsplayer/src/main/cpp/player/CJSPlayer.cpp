//
// Created by 哦 on 2023/2/13.
//

#include "CJSPlayer.h"
#include "../utils/JNICallbackHelper.h"
#include "../utils/AndroidLog.hpp"

CJSPlayer::CJSPlayer(const char *data_source, JNICallbackHelper *pHelper) {

    this->data_source = new char[strlen(data_source + 1)];
    strcpy(this->data_source, data_source);
    this->helper = pHelper;
    pthread_mutex_init(&init_mutex, nullptr);
}

CJSPlayer::~CJSPlayer() {
    if(data_source){
        delete data_source;
        data_source = nullptr;
    }
    if(helper){
        delete helper;
        helper = nullptr;
    }
    pthread_mutex_destroy(&init_mutex);

}

void* task_prepare(void * args){

    auto *player = static_cast<CJSPlayer *>(args);
    player->prepare_();

    return nullptr;
}


void CJSPlayer::prepare() {
    pthread_create(&pid_prepare, nullptr, task_prepare, this);
}

void CJSPlayer::prepare_() {
    // 准备工作
    LOGD("prepare_ url %s", data_source)
    pthread_mutex_lock(&init_mutex);
    avFormatContext = avformat_alloc_context();
    /**
     * 打开媒体地址
     */
    AVDictionary *dictionary = nullptr;
    av_dict_set(&dictionary, "timeout", "5000000", 0);

    int result = avformat_open_input(&avFormatContext, data_source, nullptr, &dictionary);
    av_dict_free(&dictionary);

    if(result){
        LOGE("open url fail : %s", data_source);
        helper->onError(THREAD_CHILD,  CODE_OPEN_URL_FAIL);
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    /**
     * 查找音视频流信息
     */
    result = avformat_find_stream_info(avFormatContext, nullptr);
    if(result){
        LOGE("can not find streams from : %s", data_source)
        helper->onError(THREAD_CHILD, CODE_FIND_STREAMS_FAIL);
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    //获取总时长
    this->duration = avFormatContext->duration / AV_TIME_BASE;
    /**
     * 获取音视频流
     */
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        AVStream *stream = avFormatContext->streams[i];
        AVCodecParameters *parameters = stream->codecpar;

        AVCodecContext *avCodecContext = nullptr;
        result = getCodecContext(parameters, &avCodecContext);
        if(result || !avCodecContext){
            LOGE("get AVCodecContext fail")
            pthread_mutex_unlock(&init_mutex);
            return;
        }
        AVRational time_base = stream->time_base;
        if(parameters->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO){
            audio_channel = new AudioChannel(i, avCodecContext, time_base);
        }
        else if(parameters->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO){

            // 特殊类型视频流，虽然是视频格式，但是只有一帧封面
            if(stream->disposition & AV_DISPOSITION_ATTACHED_PIC){
                continue;
            }
            // 获取到视频 fps
            AVRational fps_rational = stream->avg_frame_rate;

            double fps = av_q2d(fps_rational);
            video_channel = new VideoChannel(i, avCodecContext, time_base, (int)fps);
            video_channel->setRenderCallback(renderCallback);
        }
    }
    /**
     * 判断是否有音频or视频流信息
     */

    if(!audio_channel && !video_channel){
        LOGE("not media!")
        helper->onError(THREAD_CHILD, CODE_NOT_MEDIA);
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    helper->onPrepared(THREAD_CHILD);
    pthread_mutex_unlock(&init_mutex);
}

int CJSPlayer::getCodecContext(AVCodecParameters *codecPar, AVCodecContext **avCodecContext) {
    AVCodec * codec = avcodec_find_decoder(codecPar->codec_id);
    if(codec == nullptr){
        LOGE("find decoder fail!")
        helper->onError(THREAD_CHILD, CODE_FIND_DECODER_FAIL);
        return -1;
    }
    // 创建解码器上下文
    *avCodecContext = avcodec_alloc_context3(codec);
    if(*avCodecContext == nullptr){
        LOGE("alloc new AVCodecContext fail")
        helper->onError(THREAD_CHILD, CODE_ALLOC_NEW_CODEC_CONTEXT_FAIL);
        return -1;
    }
    // 将解码器参数复制到解码器上下文中
    int result = avcodec_parameters_to_context(*avCodecContext, codecPar);
    if(result){
        LOGE("fill AVCodecContext fail!");
        helper->onError(THREAD_CHILD, CODE_FILL_CODEC_CONTEXT_FAIL);
        return -1;
    }

    // 打开解码器
    result = avcodec_open2(*avCodecContext, codec, nullptr);
    if(result){
        LOGE("open codec fail");
        helper->onError(THREAD_CHILD, CODE_OPEN_CODEC_FAIL);
        return -1;
    }
    return 0;
}



void* task_start(void *args){
    auto player = static_cast<CJSPlayer *>(args);
    player->start_();

    return nullptr;
}

// 获取 音频 视频的压缩数据包(AVPacket*)并丢入队列
void CJSPlayer::start_(){
    pthread_mutex_lock(&init_mutex);
    while (isPlaying){
        // 控制队列大小，以达到优化内存的目的
        if(video_channel && video_channel->packets.size() > 100){
            av_usleep(10 * 1000); // 睡眠10毫秒
            continue;
        }
        if(audio_channel && audio_channel->packets.size() > 100){
            av_usleep(10 * 1000);
            continue;
        }

        AVPacket *packet = av_packet_alloc();
        int ret = av_read_frame(avFormatContext, packet);
        if(!ret){
            if(video_channel && video_channel->stream_index == packet->stream_index){
                video_channel->packets.insertToQueue(packet);
            }
            else if(audio_channel && audio_channel->stream_index == packet->stream_index){
                audio_channel->packets.insertToQueue(packet);
            }
        }
        else if(ret == AVERROR_EOF){
            // 数据读取完了
            // 读取完并且播放完成之后 退出
            if(video_channel && video_channel->packets.empty() && audio_channel && audio_channel->packets.empty()){
                // 播放完成之后再退出
                break;
            }
        }
        else {
            // av_read_frame 出现错误，结束循环
            break;
        }
    }
    isPlaying = 0;
    if(video_channel){
        video_channel->stop();
    }
    if(audio_channel){
        audio_channel->stop();
    }
    pthread_mutex_unlock(&init_mutex);
}

void CJSPlayer::start() {
    LOGD("start")
    isPlaying = 1;

    // 开始播放
    if(video_channel){
        video_channel->setAudioChannel(audio_channel);
        video_channel->start();
    }

    if(audio_channel){
        audio_channel->start();
    }

    // 获取音频和视频的数据压缩包，丢入队列
    pthread_create(&pid_start, nullptr, task_start, this);
}

void CJSPlayer::setRenderCallback(RenderCallback callback) {
    this->renderCallback = callback;
}

int CJSPlayer::getDuration() {
    return duration;
}

void CJSPlayer::seek(int secs) {

}
