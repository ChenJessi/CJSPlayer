//
// Created by CHEN on 2019/2/26.
//
#include <jni.h>
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

    jmethodID jmid_parpared;
    jmethodID jmid_load;
    jmethodID jmid_timeInfo;
    jmethodID jmid_error;
    jmethodID jmid_complete;
    jmethodID jmid_valumeDB;

public:
    CyCallJava(_JavaVM *javaVM, JNIEnv *env, jobject *obj);
    ~CyCallJava();

    void onCallParpared(int type);
    void onCallLoad(int type, bool load);
    void onCallTimeInfo(int type,int currentTime, int totalTime);
    void onCallError(int type,int code, char *msg);
    void onCallComplete(int type);
    void onCallValumeDB(int type, int db);
};


#endif //CYPLAYER_CYCALLJAVA_H
