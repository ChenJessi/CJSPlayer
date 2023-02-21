//
// Created by 哦 on 2023/2/15.
//

#ifndef CJSPLAYER_VIDEOCHANNEL_H
#define CJSPLAYER_VIDEOCHANNEL_H

#include "BaseChannel.h"

class VideoChannel : public BaseChannel{
public:
    VideoChannel(int stream_index, AVCodecContext *codecContext);
    ~VideoChannel();


    void start();
    void stop();

};


#endif //CJSPLAYER_VIDEOCHANNEL_H
