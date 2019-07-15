//
// Created by CHEN on 2019/7/15.
//

#ifndef CYPLAYER_SLESRECORD_H
#define CYPLAYER_SLESRECORD_H

#include <string>
#include "AndroidLog.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "RecordBuffer.h"


class SLESRecord {
public:
    SLObjectItf slObjectEngine = NULL;
    SLEngineItf engineItf = NULL;

    SLObjectItf recordObj = NULL;
    SLRecordItf recordItf = NULL;

    SLAndroidSimpleBufferQueueItf recorderBufferQueue = NULL;

    RecordBuffer *recordBuffer;

    FILE *pcmFile = NULL;

    bool finish = false;

public:
    SLESRecord();
    ~SLESRecord();

    void start();
    void stopRecord();
};


#endif //CYPLAYER_SLESRECORD_H
