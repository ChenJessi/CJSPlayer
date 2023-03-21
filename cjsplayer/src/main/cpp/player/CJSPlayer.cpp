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
    pthread_mutex_init(&seek_mutex, nullptr);
}

CJSPlayer::~CJSPlayer() {
    LOGE("CJSPlayer 析构函数")
    if(data_source){
        delete data_source;
        data_source = nullptr;
    }
    if(helper){
        delete helper;
        helper = nullptr;
    }
    pthread_mutex_destroy(&init_mutex);
    pthread_mutex_destroy(&seek_mutex);

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
        LOGE("open url fail : %s, %d", data_source, result);
        if(helper){
            helper->onError(THREAD_CHILD, CODE_OPEN_URL_FAIL);
        }
        avformat_close_input(&avFormatContext);
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    /**
     * 查找音视频流信息
     */
    result = avformat_find_stream_info(avFormatContext, nullptr);
    if(result){
        LOGE("can not find streams from : %s", data_source)
        if(helper){
            helper->onError(THREAD_CHILD, CODE_FIND_STREAMS_FAIL);
        }
        avformat_close_input(&avFormatContext);
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    //获取总时长
    this->duration = avFormatContext->duration / AV_TIME_BASE;
    /**
     * 获取音视频流
     */
    AVCodecContext *avCodecContext = nullptr;
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        AVStream *stream = avFormatContext->streams[i];
        AVCodecParameters *parameters = stream->codecpar;

        result = getCodecContext(parameters, &avCodecContext);
        if(result || !avCodecContext){
            LOGE("get AVCodecContext fail")

            avcodec_free_context(&avCodecContext);
            avformat_close_input(&avFormatContext);

            pthread_mutex_unlock(&init_mutex);
            return;
        }
        AVRational time_base = stream->time_base;
        if(parameters->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO){
            audio_channel = new AudioChannel(i, avCodecContext, time_base);
            if(duration != 0){
                audio_channel->setJNICallbackHelper(helper);
            }
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
            if(duration != 0){
                video_channel->setJNICallbackHelper(helper);
            }
        }
    }
    /**
     * 判断是否有音频or视频流信息
     */

    if(!audio_channel && !video_channel){
        LOGE("not media!")
        if (helper){
            helper->onError(THREAD_CHILD, CODE_NOT_MEDIA);
        }

        avformat_close_input(&avFormatContext);
        avFormatContext = nullptr;
        avcodec_free_context(&avCodecContext);
        pthread_mutex_unlock(&init_mutex);
        return;
    }
    if (helper){
        helper->onPrepared(THREAD_CHILD);
    }
    pthread_mutex_unlock(&init_mutex);
}

int CJSPlayer::getCodecContext(AVCodecParameters *codecPar, AVCodecContext **avCodecContext) {
    AVCodec * codec = avcodec_find_decoder(codecPar->codec_id);
    if(codec == nullptr){
        LOGE("find decoder fail!")
        if (helper){
            helper->onError(THREAD_CHILD, CODE_FIND_DECODER_FAIL);
        }
        return -1;
    }
    // 创建解码器上下文
    *avCodecContext = avcodec_alloc_context3(codec);
    if(*avCodecContext == nullptr){
        LOGE("alloc new AVCodecContext fail")
        if (helper){
            helper->onError(THREAD_CHILD, CODE_ALLOC_NEW_CODEC_CONTEXT_FAIL);
        }
        return -1;
    }
    // 将解码器参数复制到解码器上下文中
    int result = avcodec_parameters_to_context(*avCodecContext, codecPar);
    if(result){
        LOGE("fill AVCodecContext fail!");
        if (helper){
            helper->onError(THREAD_CHILD, CODE_FILL_CODEC_CONTEXT_FAIL);
        }
        return -1;
    }

    // 打开解码器
    result = avcodec_open2(*avCodecContext, codec, nullptr);
    if(result){
        LOGE("open codec fail");
        if (helper){
            helper->onError(THREAD_CHILD, CODE_OPEN_CODEC_FAIL);
        }
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
            } else{
                LOGE("packet stream_index error")
            }

        }
        else if(ret == AVERROR_EOF){
            // 数据读取完了
            // 读取完并且播放完成之后 退出
            if(video_channel && video_channel->packets.empty() && audio_channel && audio_channel->packets.empty()){
                // 播放完成之后再退出
                break;
            }
            LOGE("packet stream_index error >>>>>>")
        }
        else {
            // av_read_frame 出现错误，结束循环
            LOGE("packet stream_index error ")
            break;
        }
    }
    isPlaying = 0;
    if(audio_channel){
        audio_channel->stop();
    }

    if(video_channel){
        video_channel->stop();
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

    LOGE("seek %d", secs)
    if(secs < 0 || secs > duration){
        return;
    }
    if(!audio_channel && !audio_channel){
        return;
    }

    if(!avFormatContext){
        return;
    }

    pthread_mutex_lock(&seek_mutex);
    /**
     * 1.formatContext
     * 2.-1 代表默认情况，FFmpeg自动选择 音频 还是 视频 做 seek，  模糊：0视频  1音频
     * 3. AVSEEK_FLAG_ANY（老实） 直接精准到 拖动的位置，问题：如果不是关键帧，B帧 可能会造成 花屏情况
     *    AVSEEK_FLAG_BACKWARD（则优  8的位置 B帧 ， 找附件的关键帧 6，如果找不到他也会花屏）
     *    AVSEEK_FLAG_FRAME 找关键帧（非常不准确，可能会跳的太多），一般不会直接用，但是会配合用
     *    AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME
     */
    int result = av_seek_frame(avFormatContext, -1, secs * AV_TIME_BASE, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
    //avformat_seek_file(avFormatContext, -1 , INT64_MIN, secs * AV_TIME_BASE, INT64_MAX, 0);
    if(result < 0){
        // seek 失败
        return;
    }

    if(audio_channel){
        audio_channel->packets.setWork(0);  // 队列不工作
        audio_channel->frames.setWork(0);  // 队列不工作
        audio_channel->packets.clear();
        audio_channel->frames.clear();
        audio_channel->packets.setWork(1); // 队列继续工作
        audio_channel->frames.setWork(1);  // 队列继续工作
    }

    if (video_channel) {
        video_channel->packets.setWork(0);  // 队列不工作
        video_channel->frames.setWork(0);  // 队列不工作
        video_channel->packets.clear();
        video_channel->frames.clear();
        video_channel->packets.setWork(1); // 队列继续工作
        video_channel->frames.setWork(1);  // 队列继续工作
    }

    pthread_mutex_unlock(&seek_mutex);
}



void *task_stop(void *args) {
    auto player = static_cast<CJSPlayer *>(args);
    player->stop_(player);
    return nullptr;
}


void CJSPlayer::stop() {
    LOGD("CJSPlayer stop")


    if(audio_channel && audio_channel->jniCallbackHelper){
        audio_channel->jniCallbackHelper = nullptr;
    }
    if (video_channel && video_channel->jniCallbackHelper){
        video_channel->jniCallbackHelper = nullptr;
    }
    if (helper){
        delete helper;
        helper = nullptr;
    }
    pthread_create(&pid_stop, nullptr, task_stop, this);

    pthread_join(pid_stop, nullptr);
    if (avFormatContext){
        avformat_close_input(&avFormatContext);
        avFormatContext = nullptr;
    }

    DELETE(audio_channel)
    DELETE(video_channel)
}

void CJSPlayer::stop_(CJSPlayer *player) {

    isPlaying = false;
    pthread_join(pid_prepare, nullptr);
    pthread_join(pid_start, nullptr);

}
