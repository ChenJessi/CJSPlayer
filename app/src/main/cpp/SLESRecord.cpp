//
// Created by CHEN on 2019/7/15.
//

#include "SLESRecord.h"

void bqRecorderCallback(SLAndroidSimpleBufferQueueItf bq, void * context) {
    SLESRecord *slesRecord=(SLESRecord *)(context);

    fwrite(slesRecord->recordBuffer->getNowBuffer(), 1, 4096, slesRecord->pcmFile);
    if (slesRecord->finish) {
        LOGE("录制完成");
        (*slesRecord->recordItf)->SetRecordState(slesRecord->recordItf, SL_RECORDSTATE_STOPPED);

        (*slesRecord->recordObj)->Destroy(slesRecord->recordObj);
        slesRecord->recordObj = NULL;
        slesRecord->recordItf = NULL;
        (*slesRecord->slObjectEngine)->Destroy(slesRecord->slObjectEngine);
        slesRecord->slObjectEngine = NULL;
        slesRecord->engineItf = NULL;
        delete (slesRecord->recordBuffer);
    }
}

void SLESRecord::start() {
    recordBuffer = new RecordBuffer(4096);

    slCreateEngine(&slObjectEngine, 0, NULL, 0, NULL, NULL);
    (*slObjectEngine)->Realize(slObjectEngine, SL_BOOLEAN_FALSE);
    (*slObjectEngine)->GetInterface(slObjectEngine, SL_IID_ENGINE, &engineItf);

    SLDataLocator_IODevice loc_dev = {SL_DATALOCATOR_IODEVICE,
                                      SL_IODEVICE_AUDIOINPUT,
                                      SL_DEFAULTDEVICEID_AUDIOINPUT,
                                      NULL};
    SLDataSource audioSrc = {&loc_dev, NULL};
    SLDataLocator_AndroidSimpleBufferQueue loc_bq = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};

    SLDataFormat_PCM format_pcm = {
            SL_DATAFORMAT_PCM, 2 , SL_SAMPLINGRATE_44_1,
            SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN
    };

    SLDataSink audioSnk = {&loc_bq, &format_pcm};

    const SLInterfaceID id[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};

    (*engineItf)->CreateAudioRecorder(engineItf, &recordObj, &audioSrc, &audioSnk,1, id, req);
    (*recordObj)->Realize(recordObj, SL_BOOLEAN_FALSE);
    (*recordObj)->GetInterface(recordObj, SL_IID_RECORD, &recordItf);

    (*recordObj)->GetInterface(recordObj, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &recorderBufferQueue);

    (*recorderBufferQueue)->Enqueue(recorderBufferQueue, recordBuffer->getRecordBuffer(), 4096);
    (*recorderBufferQueue)->RegisterCallback(recorderBufferQueue, bqRecorderCallback, this);

    (*recordItf)->SetRecordState(recordItf, SL_RECORDSTATE_RECORDING);

}

void SLESRecord::stopRecord() {
    finish = true;
}

SLESRecord::SLESRecord() {

}

SLESRecord::~SLESRecord() {

}





