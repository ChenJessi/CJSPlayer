#include <jni.h>
#include <string>
#include "AndroidLog.h"
#include "CyCallJava.h"
#include "CyFFmpeg.h"
#include "SLESRecord.h"

extern "C"
{
#include <libavformat/avformat.h>
#import <SLES/OpenSLES.h>
#import <SLES/OpenSLES_Android.h>

}


_JavaVM *javaVM = NULL;
CyCallJava *callJava = NULL;
CyFFmpeg *fFmpeg = NULL;
CyPlaystatus *playstatus = NULL;
bool nexit = true;
pthread_t thread_start;


extern "C"
JNIEXPORT void JNICALL
Java_com_chen_cyplayer_player_CyPlayer_n_1prepared(JNIEnv *env, jobject instance, jstring source_) {
    const char *source = env->GetStringUTFChars(source_, 0);

    if (fFmpeg == NULL) {
        if (callJava == NULL) {
            callJava = new CyCallJava(javaVM, env, &instance);
        }
        callJava->onCallLoad(MAIN_THREAD, true);
        playstatus = new CyPlaystatus();
        fFmpeg = new CyFFmpeg(playstatus, callJava, source);
        fFmpeg->prepared();
    }
}

void *startCallBack(void *data) {
    CyFFmpeg *cyFFmpeg = (CyFFmpeg *) data;
    cyFFmpeg->start();
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_chen_cyplayer_player_CyPlayer_n_1start(JNIEnv *env, jobject instance) {
    // TODO
    if (fFmpeg != NULL) {
        pthread_create(&thread_start, NULL, startCallBack, fFmpeg);
    }
}

extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    jint result = -1;
    javaVM = vm;
    JNIEnv *env;
    if (vm->GetEnv((void **) (&env), JNI_VERSION_1_4) != JNI_OK) {
        return result;
    }
    return JNI_VERSION_1_4;
};

extern "C"
JNIEXPORT void JNICALL
Java_com_chen_cyplayer_player_CyPlayer_n_1pause(JNIEnv *env, jobject instance) {

    // TODO
    if (fFmpeg != NULL) {
        fFmpeg->pause();
    }

}


extern "C"
JNIEXPORT void JNICALL
Java_com_chen_cyplayer_player_CyPlayer_n_1resume(JNIEnv *env, jobject instance) {
    // TODO
    if (fFmpeg != NULL) {
        fFmpeg->resume();
    }

}


extern "C"
JNIEXPORT void JNICALL
Java_com_chen_cyplayer_player_CyPlayer_n_1stop(JNIEnv *env, jobject instance) {
    // TODO
    if (!nexit) {
        return;
    }
    jclass clz = env->GetObjectClass(instance);
    jmethodID jmid_next = env->GetMethodID(clz, "onCallNext", "()V");
    nexit = false;
    if (fFmpeg != NULL) {
        fFmpeg->release();
        pthread_join(thread_start, NULL);
        delete (fFmpeg);
        LOGD("fFmpeg 释放")
        fFmpeg = NULL;
        if (callJava != NULL) {
            delete (callJava);
            callJava = NULL;
            LOGD("callJava 释放")
        }
        if (playstatus != NULL) {
            delete (playstatus);
            playstatus = NULL;
            LOGD("playstatus 释放")
        }
    }
    nexit = true;
    env->CallVoidMethod(instance, jmid_next);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_chen_cyplayer_player_CyPlayer_n_1seek(JNIEnv *env, jobject instance, jint secds) {

    // TODO
    if (fFmpeg != NULL) {
        LOGE("seek 1time %d", secds);
        fFmpeg->seek(secds);
    }

}

extern "C"
JNIEXPORT jint JNICALL
Java_com_chen_cyplayer_player_CyPlayer_n_1duration(JNIEnv *env, jobject instance) {
    // TODO
    if (fFmpeg != NULL) {
        return fFmpeg->duration;
    }
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_chen_cyplayer_player_CyPlayer_n_1volume(JNIEnv *env, jobject instance, jint percent) {

    // TODO
    if (fFmpeg != NULL) {
        fFmpeg->setVolume(percent);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_chen_cyplayer_player_CyPlayer_n_1mute(JNIEnv *env, jobject instance, jint mute) {
    // TODO
    if (fFmpeg != NULL) {
        fFmpeg->setMute(mute);
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_com_chen_cyplayer_player_CyPlayer_n_1pitch(JNIEnv *env, jobject instance, jfloat pitch) {
    // TODO
    if (fFmpeg != NULL) {
        fFmpeg->setPitch(pitch);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_chen_cyplayer_player_CyPlayer_n_1speed(JNIEnv *env, jobject instance, jfloat speed) {
    // TODO
    if (fFmpeg != NULL) {
        fFmpeg->setSpeed(speed);
    }
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_chen_cyplayer_player_CyPlayer_n_1samplerate(JNIEnv *env, jobject instance) {
    // TODO
    if (fFmpeg != NULL) {
        return fFmpeg->getSampleRate();
    }
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_chen_cyplayer_player_CyPlayer_n_1startstoprecord(JNIEnv *env, jobject instance,
                                                          jboolean start) {
    // TODO
    if (fFmpeg != NULL) {
        fFmpeg->startStopRecord(start);
    }
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_chen_cyplayer_player_CyPlayer_n_1cutaudioplay(JNIEnv *env, jobject instance,
                                                       jint start_time, jint end_time,
                                                       jboolean showPcm) {
    // TODO
    if (fFmpeg != NULL) {
        return fFmpeg->cutAudioPlay(start_time, end_time, showPcm);
    }
    return false;
}


//#include <SLES/OpenSLES.h>
//#include <SLES/OpenSLES_Android.h>
//#include "RecordBuffer.h"
//#include "AndroidLog.h"

SLObjectItf slObjectEngine = NULL;
SLEngineItf  engineItf = NULL;

SLObjectItf  recordObj = NULL;
SLRecordItf  recordItf = NULL;

SLAndroidSimpleBufferQueueItf recorderBufferQueue = NULL;

RecordBuffer *recordBuffer;

FILE *pcmFile = NULL;

bool finish = false;


void bqRecorderCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    fwrite(recordBuffer->getNowBuffer(), 1, 4096, pcmFile);
    if(finish)
    {
        LOGD("录制完成");
        (*recordItf)->SetRecordState(recordItf, SL_RECORDSTATE_STOPPED);
        //
        (*recordObj)->Destroy(recordObj);
        recordObj = NULL;
        recordItf = NULL;
        (*slObjectEngine)->Destroy(slObjectEngine);
        slObjectEngine = NULL;
        engineItf = NULL;
        delete(recordBuffer);
    } else{
        LOGD("正在录制");
        (*recorderBufferQueue)->Enqueue(recorderBufferQueue, recordBuffer->getRecordBuffer(), 4096);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_chen_cyplayer_RecordActivity_startRecord(JNIEnv *env, jobject instance, jstring path_) {


    // TODO
//    if (slesRecord != NULL){
//        return;
//    }
//    slesRecord = new SLESRecord();
//    if (slesRecord->finish){
//        return;
//    }
//    const char *path = env->GetStringUTFChars(path_, 0);
//    slesRecord->finish = false;
//    slesRecord->pcmFile - fopen(path,"w");
//    slesRecord->start();
//
//    env->ReleaseStringUTFChars(path_, path);
    if(finish)
    {
        return;
    }
    const char *path = env->GetStringUTFChars(path_, 0);
    finish = false;
    pcmFile = fopen(path, "w");
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
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
            2
    };


    SLDataFormat_PCM format_pcm = {
            SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1,
            SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN
    };

    SLDataSink audioSnk = {&loc_bq, &format_pcm};

    const SLInterfaceID id[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};

    (*engineItf)->CreateAudioRecorder(engineItf, &recordObj, &audioSrc, &audioSnk, 1, id, req);
    (*recordObj)->Realize(recordObj, SL_BOOLEAN_FALSE);
    (*recordObj)->GetInterface(recordObj, SL_IID_RECORD, &recordItf);

    (*recordObj)->GetInterface(recordObj, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &recorderBufferQueue);


    (*recorderBufferQueue)->Enqueue(recorderBufferQueue, recordBuffer->getRecordBuffer(), 4096);

    (*recorderBufferQueue)->RegisterCallback(recorderBufferQueue, bqRecorderCallback, NULL);

    (*recordItf)->SetRecordState(recordItf, SL_RECORDSTATE_RECORDING);

    env->ReleaseStringUTFChars(path_, path);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_chen_cyplayer_RecordActivity_stopRecord(JNIEnv *env, jobject instance) {
//    slesRecord->stopRecord();
    // TODO
    finish = true;

}


#include "push/RtmpPush.h"

RtmpPush * rtmpPush = NULL;
//
//push rtmp 推流
//
 extern "C"
JNIEXPORT void JNICALL
Java_com_chen_cyplayer_push_CyPushVideo_initPush(JNIEnv *env, jobject instance, jstring pushUrl_) {
    const char *pushUrl = env->GetStringUTFChars(pushUrl_, 0);

    // TODO
    rtmpPush = new RtmpPush(pushUrl);
    rtmpPush->init();

    env->ReleaseStringUTFChars(pushUrl_, pushUrl);
}