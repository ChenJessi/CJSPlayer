//
// Created by 哦 on 2023/2/15.
//

#ifndef CJSPLAYER_VIDEOCHANNEL_H
#define CJSPLAYER_VIDEOCHANNEL_H

#include "BaseChannel.h"
#include "../utils/AndroidLog.hpp"
#include "AudioChannel.h"
extern "C"{
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavutil/avutil.h"
};

// 函数指针，用于回调视频数据进行播放
// 视频数据，宽 高 和步长
typedef void(*RenderCallback)(uint8_t *, int, int, int);

class VideoChannel : public BaseChannel{
public:
    VideoChannel(int stream_index, AVCodecContext *codecContext, AVRational time_base, int fps);
    ~VideoChannel();


    void start();
    void stop();

    void video_decode();
    void video_play();

    void setRenderCallback(RenderCallback renderCallback);

    void setAudioChannel(AudioChannel *audio_channel);
private:
    bool isPlaying = false;

    pthread_t pid_video_decode;
    pthread_t pid_video_play;

    RenderCallback renderCallback;
    AudioChannel *audioChannel = nullptr;

    int fps = 0;

};


#endif //CJSPLAYER_VIDEOCHANNEL_H
