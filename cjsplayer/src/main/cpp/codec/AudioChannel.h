//
// Created by 哦 on 2023/2/15.
//

#ifndef CJSPLAYER_AUDIOCHANNEL_H
#define CJSPLAYER_AUDIOCHANNEL_H

#include "BaseChannel.h"

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "../utils/AndroidLog.hpp"

extern "C"{
#include "libswresample/swresample.h" //    音频冲采样
#include "libavutil/avutil.h"
};


class AudioChannel : public BaseChannel{

public:
    AudioChannel(int stream_index, AVCodecContext *codecContext, AVRational time_base);
    ~AudioChannel();


    void start();
    void stop();

    void audio_decode();
    void audio_play();

    int getPCM();

    // 缓冲区
    uint8_t *out_buffers = nullptr;

    // 音频播放的时间戳
    double audio_time = 0;
private:
    bool isPlaying = false;

    pthread_t pid_audio_decode;
    pthread_t pid_audio_play;


    // 输出的通道数
    int out_channels = 2;
    // 样本大小
    int out_sample_size = 0;
    // 采样率
    int out_sample_rate = 0;
    // 缓冲区大小
    int out_buffers_size = 0;

    // 冲采样上下文
    SwrContext *swr_ctx = nullptr;



    // 引擎
    SLObjectItf engineObject = nullptr;
    // 引擎接口
    SLEngineItf engineInterface = nullptr;
    // 混音器
    SLObjectItf outputMixObject = nullptr;
    // 混音器接口
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = nullptr;
    // 播放器
    SLObjectItf bqPlayerObject = nullptr;
    // 播放器接口
    SLPlayItf bqPlayerPlay = nullptr;
    // 播放器队列接口
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue = nullptr;
};

#endif //CJSPLAYER_AUDIOCHANNEL_H
