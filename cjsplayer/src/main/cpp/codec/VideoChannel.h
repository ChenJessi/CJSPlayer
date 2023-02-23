//
// Created by 哦 on 2023/2/15.
//

#ifndef CJSPLAYER_VIDEOCHANNEL_H
#define CJSPLAYER_VIDEOCHANNEL_H

#include "BaseChannel.h"

extern "C"{
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
};

class VideoChannel : public BaseChannel{
public:
    VideoChannel(int stream_index, AVCodecContext *codecContext);
    ~VideoChannel();


    void start();
    void stop();

    void video_decode();
    void video_play();
private:
    bool isPlaying = false;

    pthread_t pid_video_decode;
    pthread_t pid_video_play;


};


#endif //CJSPLAYER_VIDEOCHANNEL_H
