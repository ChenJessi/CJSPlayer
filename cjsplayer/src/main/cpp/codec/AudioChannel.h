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
};

#endif //CJSPLAYER_AUDIOCHANNEL_H
