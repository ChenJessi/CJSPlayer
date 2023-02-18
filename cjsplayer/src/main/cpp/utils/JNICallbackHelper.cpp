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
}

void JNICallbackHelper::onPrepared(int thread_mode) {

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

void JNICallbackHelper::onError(int thread_mode) {

}
