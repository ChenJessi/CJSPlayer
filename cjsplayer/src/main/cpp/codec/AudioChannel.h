//
// Created by 哦 on 2023/2/15.
//

#ifndef CJSPLAYER_AUDIOCHANNEL_H
#define CJSPLAYER_AUDIOCHANNEL_H

#include "BaseChannel.h"
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
};

#endif //CJSPLAYER_AUDIOCHANNEL_H
