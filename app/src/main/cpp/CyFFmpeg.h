//
// Created by CHEN on 2019/2/26.
//
#include "pthread.h"
#include "CyCallJava.h"
#include "CyAudio.h"
#include "CyVideo.h"

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
    CyVideo *video = NULL;
    CyPlaystatus *playstatus = NULL;

    pthread_mutex_t init_mutex;
    bool exit = false;

    pthread_mutex_t seek_mutex;
    int duration = 0;
public:
    CyFFmpeg( CyPlaystatus *playstatus, CyCallJava *callJava, const char *url);
    ~CyFFmpeg();

    void prepared();
    void decodeFFmpegThread();
    void start();

    void pause();
    void resume();
    void stop();
    void release();

    int getCodecContext(AVCodecParameters *codecpar , AVCodecContext **avCodecContext);


    void seek(int64_t secds);

    void setVolume(int percent);

    void setMute(int mute);

    void setPitch(float pitch);

    void setSpeed(float speed);

    int getSampleRate();

    void startStopRecord(bool start);

    bool cutAudioPlay(int start_time, int end_time, bool showPcm);


};


#endif //CYPLAYER_CYFFMPEG_H
