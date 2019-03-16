//
// Created by CHEN on 2019/2/26.
//
#include "CyPlaystatus.h"
#include "CyQueue.h"
#include "CyCallJava.h"

#include "SoundTouch.h"
extern "C"{
#include "libavcodec/avcodec.h"
#import <libswresample/swresample.h>
#include "libavcodec/avcodec.h"
#import <SLES/OpenSLES.h>
#import <SLES/OpenSLES_Android.h>
#include <libavutil/time.h>
}

using namespace soundtouch;
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

    int volumePercent = 50;
    int mute = 2;

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
    SLMuteSoloItf  pcmMutePlay = NULL;

    //缓冲队列接口
    SLAndroidSimpleBufferQueueItf  pcmBufferQueue = NULL;

    //soundTouch
    uint8_t  *out_buffer = NULL;
    bool  finished = true;
    SoundTouch *soundTouch = NULL;
    SAMPLETYPE  *sampleBuffer = NULL;
    int nb = 0;
    int num = 0;
    float pitch = 1.0f;
    float speed = 1.0f;

    pthread_mutex_t sound_mutex;

    bool isRecordPcm = false;
    bool readFrameFinished = true;

    bool isCut = false;
    int end_time = 0;
    bool showPcm = false;
public:
    CyAudio(CyPlaystatus *cyPlaystatus, int sample_rate ,CyCallJava *callJava);
    ~CyAudio();

    void play();
    int resampleAudio(void **pcmbuf);

    void initOpenSLES();

    SLuint32 getCurrentSampleRateForOpensles(int sample_rate);

    void pause();
    void resume();

    void stop();
    void release();

    void setVolume(int percent);

    void setMute(int mute);

    int getSoundTouchData();

    void setPitch(float pitch);

    void setSpeed(float speed);

    int getPCMDB(char *pcmdata, size_t pcmsize);

    void startStopRecord(bool start);
};



#endif //CYPLAYER_CYAUDIO_H
