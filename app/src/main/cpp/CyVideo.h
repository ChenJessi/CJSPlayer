//
// Created by CHEN on 2019/3/18.
//

#ifndef CYPLAYER_CYVIDEO_H
#define CYPLAYER_CYVIDEO_H
#include "CyQueue.h"
#include "CyCallJava.h"
#include "CyAudio.h"

#define CODEC_YUV 0
#define CODEC_MEDIACODEC 1
extern "C"
{
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include "libavutil/time.h"
#include <libavutil/imgutils.h>
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

    CyAudio *audio = NULL;
    double clock = 0;
    double delayTime = 0;
    double defaultDelayTime = 0.04;
    pthread_mutex_t codecMutex;

    int codectype = CODEC_YUV;
    AVBSFContext *abs_ctx = NULL;

public:
    CyVideo( CyPlaystatus *playstatus, CyCallJava *callJava);
   ~CyVideo();
    void play();
    void release();

    double getFrameDiffTime(AVFrame *avFrame ,  AVPacket *avPacket);
    double getDelayTime(double diff);
};


#endif //CYPLAYER_CYVIDEO_H
