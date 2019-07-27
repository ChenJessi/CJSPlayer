//
// Created by CHEN on 2019/7/27.
//



#include <jni.h>
#include <cstddef>
#include "../AndroidLog.h"
#ifndef CYPLAYER_CYPUSHCALLJAVA_H
#define CYPLAYER_CYPUSHCALLJAVA_H
#define MAIN_THREAD 0
#define CHILD_THREAD 1

class CyPushCallJava {
public:
    _JavaVM *javaVM = NULL;
    JNIEnv *jniEnv = NULL;
    jobject jobj;

    //    push
    jmethodID jmid_connecting;
    jmethodID jmid_connectsuccess;
    jmethodID jmid_connectfail;

public:
    CyPushCallJava(_JavaVM *javaVM, JNIEnv *env, jobject *obj);
    ~CyPushCallJava();



    void onConnectint(int type);

    void onConnectsuccess();

    void onConnectFail(char *msg);
};


#endif //CYPLAYER_CYPUSHCALLJAVA_H
