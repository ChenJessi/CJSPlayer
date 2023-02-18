//
// Created by 哦 on 2023/2/13.
//

#include "CJSPlayer.h"
#include "../utils/JNICallbackHelper.h"

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
    //avformat_open_input()

    auto *player = static_cast<CJSPlayer *>(args);
    player->prepare_();

    return 0;
}


void CJSPlayer::prepare() {
    pthread_create(&pid_prepare, 0, task_prepare, this);
}

void CJSPlayer::prepare_() {
    // 准备工作
    LOGD("prepare_ url %s", data_source)
    pthread_mutex_lock(&init_mutex);

    avformat_network_init();
    avFormatContext = avformat_alloc_context();
    /**
     * 打开媒体地址
     */
    AVDictionary *dictionary = nullptr;
    av_dict_set(&dictionary, "timeout", "5000000", 0);

    int result = avformat_open_input(&avFormatContext, data_source, nullptr, &dictionary);
    av_dict_free(&dictionary);

    if(result){
        LOGE("打开媒体文件失败 : %s", data_source);
        helper->onError(THREAD_CHILD);
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    /**
     * 查找音视频流信息
     */
    result = avformat_find_stream_info(avFormatContext, nullptr);
    if(result){
        LOGE("can not find streams from : %s", data_source)
        helper->onError(THREAD_CHILD);
        pthread_mutex_unlock(&init_mutex);
        return;
    }

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
            helper->onError(THREAD_CHILD);
            pthread_mutex_unlock(&init_mutex);
            return;
        }
        if(parameters->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO){
            audio_channel = new AudioChannel();
        }
        else if(parameters->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO){
            video_channel = new VideoChannel();
        }
    }
    /**
     * 判断是否有音频or视频流信息
     */

    if(!audio_channel && !video_channel){
        LOGE("未找到音视频流")
        helper->onError(THREAD_CHILD);
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    helper->onPrepared(THREAD_CHILD);
    pthread_mutex_unlock(&init_mutex);
}

int CJSPlayer::getCodecContext(AVCodecParameters *codecPar, AVCodecContext **avCodecContext) {
    AVCodec * codec = avcodec_find_decoder(codecPar->codec_id);
    if(codec == nullptr){
        LOGE("can not find decoder!")
        return -1;
    }
    // 创建解码器上下文
    *avCodecContext = avcodec_alloc_context3(codec);
    if(*avCodecContext == nullptr){
        LOGE("can not alloc new AVCodecContext")
        return -1;
    }
    // 将解码器参数复制到解码器上下文中
    int result = avcodec_parameters_to_context(*avCodecContext, codecPar);
    if(result){
        LOGE("can not fill AVCodecContext!");
        return -1;
    }

    // 打开解码器
    result = avcodec_open2(*avCodecContext, codec, nullptr);
    if(result){
        LOGE("open codec fail");
        return -1;
    }
    return 0;
}
