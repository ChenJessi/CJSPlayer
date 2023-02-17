//
// Created by 哦 on 2023/2/17.
//

#ifndef CJSPLAYER_JNICALLBACKHELPER_H
#define CJSPLAYER_JNICALLBACKHELPER_H


#include <jni.h>

class JNICallbackHelper {

public:
    JNICallbackHelper(_JavaVM *pVm, _JNIEnv *pEnv, jobject pJob);
    ~JNICallbackHelper();


    void onPrepared();
private:
    JavaVM *vm = nullptr;
    JNIEnv *env = nullptr;
    jobject job = nullptr;

    void initCallback();

    jmethodID jmd_prepared = nullptr;
};


#endif //CJSPLAYER_JNICALLBACKHELPER_H
