//
// Created by 哦 on 2023/2/15.
//

#ifndef CJSPLAYER_AUDIOCHANNEL_H
#define CJSPLAYER_AUDIOCHANNEL_H

#include "BaseChannel.h"

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "../utils/AndroidLog.hpp"

class AudioChannel : public BaseChannel{

public:
    AudioChannel(int stream_index, AVCodecContext *codecContext);
    ~AudioChannel();


    void start();
    void stop();

    void audio_decode();
    void audio_play();
private:
    bool isPlaying = false;

    pthread_t pid_audio_decode;
    pthread_t pid_audio_play;

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
};

#endif //CJSPLAYER_AUDIOCHANNEL_H
