//
// Created by CHEN on 2019/2/26.
//
#include "CyPlaystatus.h"
#include "CyQueue.h"
#include "CyCallJava.h"

extern "C"{
#include "libavcodec/avcodec.h"
#import <libswresample/swresample.h>
#include "libavcodec/avcodec.h"
#import <SLES/OpenSLES.h>
#import <SLES/OpenSLES_Android.h>
}

#ifndef CYPLAYER_CYAUDIO_H
#define CYPLAYER_CYAUDIO_H


class CyAudio {

public:
    int streamIndex = -1;
    AVCodecContext *avCodecContext = NULL;   //解码器上下文
    AVCodecParameters *codecpar = NULL;
    CyQueue *queue = NULL;
    CyPlaystatus *cyPlaystatus;
    CyCallJava *callJava = NULL;

    pthread_t thread_play;
    AVPacket *avPacket =NULL;
    AVFrame *avFrame = NULL;
    int ret = 0;
    uint8_t  *buffer = NULL;
    int data_size = 0;
    int sample_rate = 0;

    int duration = 0;
    AVRational time_base;
    double clock;   // 播放时长
    double now_time;  // 当前frame时间
    double last_time; //上一次调用时间

    int volumePercent = 100;
    //引擎接口
    SLObjectItf  engineObject = NULL;
    SLEngineItf  engineItf = NULL;

    //混音器
    SLObjectItf outputMixObject = NULL;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
    SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

    //PCM
    SLObjectItf  pcmPlayerObject = NULL;
    SLPlayItf  pcmPlayerPlay = NULL;
    SLVolumeItf pcmVolumePlay = NULL;

    //缓冲队列接口
    SLAndroidSimpleBufferQueueItf  pcmBufferQueue = NULL;

public:
    CyAudio(CyPlaystatus *cyPlaystatus, int sample_rate ,CyCallJava *callJava);
    ~CyAudio();

    void play();
    int resampleAudio();

    void initOpenSLES();

    SLuint32 getCurrentSampleRateForOpensles(int sample_rate);

    void pause();
    void resume();

    void stop();
    void release();

    void setVolume(int percent);
};



#endif //CYPLAYER_CYAUDIO_H
