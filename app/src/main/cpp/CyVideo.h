//
// Created by CHEN on 2019/3/18.
//

#ifndef CYPLAYER_CYVIDEO_H
#define CYPLAYER_CYVIDEO_H
#include "CyQueue.h"
#include "CyCallJava.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include "libavutil/time.h"
};

class CyVideo {
public:
    int streamIndex = -1;
    AVCodecContext *avCodecContext = NULL;
    AVCodecParameters *codecpar = NULL;
    CyQueue *queue = NULL;
    CyPlaystatus *playstatus = NULL;
    CyCallJava *callJava = NULL;
    AVRational time_base;

    pthread_t  pthread_play;

public:
    CyVideo( CyPlaystatus *playstatus, CyCallJava *callJava);
   ~CyVideo();
    void play();
    void release();

};


#endif //CYPLAYER_CYVIDEO_H
