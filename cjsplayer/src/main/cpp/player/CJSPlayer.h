//
// Created by 哦 on 2023/2/13.
//

#ifndef CJSPLAYER_CJSPLAYER_H
#define CJSPLAYER_CJSPLAYER_H

#include <cstring>
#include "pthread.h"

#include "../codec/AudioChannel.h"
#include "../codec/VideoChannel.h"
#include "../utils/AndroidLog.hpp"
#include "../utils/JNICallbackHelper.h"
#include "../utils/utils.h"
extern "C"{
    #include "libavformat/avformat.h"
};
class CJSPlayer {

public:
    CJSPlayer(const char *data_source, JNICallbackHelper *pHelper);
    ~CJSPlayer();

    void prepare();
    void prepare_();

    void start();
    void start_();


private:
    char *data_source = nullptr;
    JNICallbackHelper *helper = nullptr;
    pthread_t pid_prepare = 0;
    pthread_t pid_start = 0;

    pthread_mutex_t init_mutex;

    AVFormatContext *avFormatContext = nullptr;

    int getCodecContext(AVCodecParameters *codecPar, AVCodecContext **avCodecContext);

    AudioChannel *audio_channel = nullptr;
    VideoChannel *video_channel = nullptr;

    int isPlaying = 0;

};


#endif //CJSPLAYER_CJSPLAYER_H
