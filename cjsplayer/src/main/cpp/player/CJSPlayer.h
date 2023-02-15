//
// Created by 哦 on 2023/2/13.
//

#ifndef CJSPLAYER_CJSPLAYER_H
#define CJSPLAYER_CJSPLAYER_H

#include <cstring>
#include "pthread.h"

#include "../codec/AudioChannel.h"
#include "../codec/VideoChannel.h"
extern "C"{
    #include "libavformat/avformat.h"
};
class CJSPlayer {

public:
    CJSPlayer(const char *data_source);
    ~CJSPlayer();

    void prepare();

    void prepare_();

private:
    char *data_source = nullptr;
    pthread_t pid_prepare = 0;

    pthread_mutex_t init_mutex;

    AVFormatContext *avFormatContext = nullptr;

    int getCodecContext(AVCodecParameters *codecPar, AVCodecContext **avCodecContext);
};


#endif //CJSPLAYER_CJSPLAYER_H
