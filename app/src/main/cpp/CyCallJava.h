//
// Created by CHEN on 2019/2/26.
//
#include <jni.h>
#include <cstddef>
#include "AndroidLog.h"
#ifndef CYPLAYER_CYCALLJAVA_H
#define CYPLAYER_CYCALLJAVA_H
#define MAIN_THREAD 0
#define CHILD_THREAD 1



class CyCallJava {
public:

    _JavaVM *javaVM = NULL;
    JNIEnv *jniEnv = NULL;
    jobject jobj;

    jmethodID jmid_prepared;
    jmethodID jmid_load;
    jmethodID jmid_timeInfo;
    jmethodID jmid_error;
    jmethodID jmid_complete;
    jmethodID jmid_valumeDB;
    jmethodID jmid_pcmtoaac;
    jmethodID jmid_pcminfo;
    jmethodID jmid_pcmrate;
    jmethodID jmid_renderyuv;
    jmethodID jmid_supportvideo;
    jmethodID jmid_decodeAVPacket;
    jmethodID jmid_initMediaCodec;

    //    push
    jmethodID jmid_connecting;
    jmethodID jmid_connectsuccess;
    jmethodID jmid_connectfail;
public:
    CyCallJava(_JavaVM *javaVM, JNIEnv *env, jobject *obj);
    ~CyCallJava();

    void onCallPrepared(int type);
    void onCallLoad(int type, bool load);
    void onCallTimeInfo(int type,int currentTime, int totalTime);
    void onCallError(int type,int code, char *msg);
    void onCallComplete(int type);
    void onCallValumeDB(int type, int db);
    void onCallPcmToAAC(int type, int size, void *buffer);
    void onCallPcmInfo(int size, void *buffer);
    void onCallPcmRate(int samplerate);
    void onCallRenderYUV(int width, int height, uint8_t *fy, uint8_t *fu, uint8_t *fv);

    bool onCallSupportVideo(const char *ffcodename);

    void onCallInitMediaCodec(const char *mime, int width, int height, int csd0_size, int csd1_size, uint8_t *csd_0, uint8_t *csd_1);
    void onCallDecodeAVPacket(int datasize, uint8_t *data);


    void onConnectint(int type);

    void onConnectsuccess();

    void onConnectFail(char *msg);
};


#endif //CYPLAYER_CYCALLJAVA_H
