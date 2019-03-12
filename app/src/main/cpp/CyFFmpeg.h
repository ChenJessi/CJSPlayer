//
// Created by CHEN on 2019/2/26.
//
#include "pthread.h"
#include "CyCallJava.h"
#include "CyAudio.h"

#ifndef CYPLAYER_CYFFMPEG_H
#define CYPLAYER_CYFFMPEG_H

extern "C"{
#include "libavformat/avformat.h"
#include <libavutil/time.h>
};

class CyFFmpeg {
public:
    CyCallJava *callJava = NULL;
    const char* url = NULL;
    pthread_t decodeThread;
    AVFormatContext *pFormatCtx = NULL;
    CyAudio *audio = NULL;
    CyPlaystatus *playstatus = NULL;

    pthread_mutex_t init_mutex;
    bool exit = false;

    pthread_mutex_t seek_mutex;
    int duration = 0;
public:
    CyFFmpeg( CyPlaystatus *playstatus, CyCallJava *callJava, const char *url);
    ~CyFFmpeg();

    void parpared();
    void decodeFFmpegThread();
    void start();

    void pause();
    void resume();

    void release();

    void seek(int64_t secds);

    void setVolume(int percent);
};


#endif //CYPLAYER_CYFFMPEG_H
