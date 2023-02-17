//
// Created by 哦 on 2023/2/17.
//

#include "JNICallbackHelper.h"

JNICallbackHelper::JNICallbackHelper(_JavaVM *vm, _JNIEnv *env, jobject pJob) {
    this->vm = vm;
    this->env = env;
    this->job = env->NewGlobalRef(pJob);
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

void JNICallbackHelper::onPrepared() {

}
