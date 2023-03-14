//
// Created by 哦 on 2023/2/17.
//

#include "JNICallbackHelper.h"

JNICallbackHelper::JNICallbackHelper(_JavaVM *vm, _JNIEnv *env, jobject pJob) {
    this->vm = vm;
    this->env = env;
    this->job = env->NewGlobalRef(pJob);
    initCallback();
}

JNICallbackHelper::~JNICallbackHelper() {
    this->vm = nullptr;
    env->DeleteGlobalRef(job);
    job = nullptr;
    env = nullptr;
}

void JNICallbackHelper::initCallback() {

    jclass clazz = env->GetObjectClass(job);
    jmd_prepared = env->GetMethodID(clazz, "onPrepared", "()V");
    jmd_error = env->GetMethodID(clazz, "onError", "(I)V");
    jmd_progress = env->GetMethodID(clazz, "onProgress", "(I)V");

}

void JNICallbackHelper::onPrepared(int thread_mode) {
    LOGE("onError %d ", thread_mode)
    if(thread_mode == THREAD_MAIN){
        env->CallVoidMethod(job, jmd_prepared);
    }
    else if(thread_mode == THREAD_CHILD){
        JNIEnv * env_child = nullptr;
        vm->AttachCurrentThread(&env_child, 0);
        env_child->CallVoidMethod(job, jmd_prepared);
        vm->DetachCurrentThread();
    }
}

void JNICallbackHelper::onError(int thread_mode, int code) {
    LOGE("onError %d %d", thread_mode, code)
    if(thread_mode == THREAD_MAIN){
        env->CallVoidMethod(job, jmd_error, code);
    }
    else if(thread_mode == THREAD_CHILD){
        JNIEnv * env_child = nullptr;
        vm->AttachCurrentThread(&env_child, 0);
        env_child->CallVoidMethod(job, jmd_error, code);
        vm->DetachCurrentThread();
    }
}

void JNICallbackHelper::onProgress(int thread_mode, int time) {
    if(thread_mode == THREAD_MAIN){
        env->CallVoidMethod(job, jmd_progress, time);
    }
    else if(thread_mode == THREAD_CHILD){
        JNIEnv * env_child = nullptr;
        vm->AttachCurrentThread(&env_child, 0);
        env_child->CallVoidMethod(job, jmd_progress, time);
        vm->DetachCurrentThread();
    }
}
