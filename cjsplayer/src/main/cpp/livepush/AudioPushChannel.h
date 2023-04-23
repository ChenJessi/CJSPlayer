//
// Created by 哦 on 2023/4/9.
//

#ifndef CJSPLAYER_AUDIOPUSHCHANNEL_H
#define CJSPLAYER_AUDIOPUSHCHANNEL_H

#include <rtmp.h>
#include <faac.h>
#include <cstring>
#include "../utils/AndroidLog.hpp"


class AudioPushChannel {

public:
    typedef void (*AudioCallback)(RTMPPacket *packet);

    void initAudioEncoder(int sample_rate, int num_channel);

public:
    AudioPushChannel();

    ~AudioPushChannel();

    void setAudioCallback(AudioCallback callback);
    int getInputSamples();

private:
    AudioCallback audioCallback;
    int mChannels = 1; // 声道数
    faacEncHandle audioEncoder = 0; // 编码器
    u_long inputSamples = 0; // 编码器每次编码的样本数
    u_long maxOutputBytes = 0; // 编码器每次编码的最大输出字节数
    u_char *buffer = 0; // 编码器输出缓冲区

};

#endif //CJSPLAYER_AUDIOPUSHCHANNEL_H
