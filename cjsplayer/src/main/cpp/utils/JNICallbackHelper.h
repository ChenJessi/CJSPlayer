//
// Created by 哦 on 2023/2/17.
//

#ifndef CJSPLAYER_JNICALLBACKHELPER_H
#define CJSPLAYER_JNICALLBACKHELPER_H


#include <jni.h>
#include "../utils/utils.h"
#include "AndroidLog.hpp"
class JNICallbackHelper {

public:
    JNICallbackHelper(_JavaVM *pVm, _JNIEnv *pEnv, jobject pJob);
    ~JNICallbackHelper();


    void onPrepared(int thread_mode);
    void onError(int thread_mode, int code);
    void onProgress(int thread_mode, int time);
private:
    JavaVM *vm = nullptr;
    JNIEnv *env = nullptr;
    jobject job = nullptr;

    void initCallback();

    jmethodID jmd_prepared = nullptr;
    jmethodID jmd_error = nullptr;
    jmethodID jmd_progress = nullptr;
};


#endif //CJSPLAYER_JNICALLBACKHELPER_H
