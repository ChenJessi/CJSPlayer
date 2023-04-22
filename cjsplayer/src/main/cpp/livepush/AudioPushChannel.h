//
// Created by 哦 on 2023/4/9.
//

#ifndef CJSPLAYER_AUDIOPUSHCHANNEL_H
#define CJSPLAYER_AUDIOPUSHCHANNEL_H

#include <rtmp.h>
#include <faac.h>


class AudioPushChannel {

public:
    typedef void (*AudioCallback)(RTMPPacket *packet);

    void initAudioEncoder(int sample_rate, int num_channel);

public:
    AudioPushChannel();

    ~AudioPushChannel();

    void setAudioCallback(AudioCallback callback);

private:
    AudioCallback audioCallback;
};

#endif //CJSPLAYER_AUDIOPUSHCHANNEL_H
