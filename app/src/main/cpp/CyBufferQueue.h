//
// Created by CHEN on 2019/3/17.
//

#ifndef CYPLAYER_CYBUFFERQUEUE_H
#define CYPLAYER_CYBUFFERQUEUE_H

#include "deque"
#include "CyPlaystatus.h"
#include "CyPcmBean.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include "pthread.h"
};

class CyBufferQueue {
public:
    std::deque<CyPcmBean *> queueBuffer;
    pthread_mutex_t mutexBuffer;
    pthread_cond_t condBuffer;
    CyPlaystatus *cyPlaystatus = NULL;

public:
    CyBufferQueue(CyPlaystatus *playstatus);
    ~CyBufferQueue();

    int putBuffer(SAMPLETYPE *buffer, int size);
    int getBuffer(CyPcmBean **pcmBean);

    int clearBuffer();

    void release();

    int noticeThread();

    int getBufferSize();
};


#endif //CYPLAYER_CYBUFFERQUEUE_H
