//
// Created by CHEN on 2019/7/27.
//

#include "CyPushCallJava.h"

CyPushCallJava::CyPushCallJava(_JavaVM *javaVM, JNIEnv *env, jobject *obj) {
    this->javaVM = javaVM;
    this->jniEnv = env;
    this->jobj = *obj;
    this->jobj = env->NewGlobalRef(jobj);

    jclass jlz = jniEnv->GetObjectClass(jobj);

    if (!jlz) {

        if (LOG_DEBUG) {
            LOGE("get jclass wrong!");
        }
        return;
    }

    //push
    jmid_connecting = env->GetMethodID(jlz, "onConnecting", "()V");
    jmid_connectsuccess = env->GetMethodID(jlz, "onConnectSuccess", "()V");
    jmid_connectfail = env->GetMethodID(jlz, "onConnectFial", "(Ljava/lang/String;)V");
}

CyPushCallJava::~CyPushCallJava() {
    jniEnv->DeleteGlobalRef(jobj);
    javaVM = NULL;
    jniEnv = NULL;
}


void CyPushCallJava::onConnectint(int type) {
    if (type == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_connecting);
        javaVM->DetachCurrentThread();
    } else {
        jniEnv->CallVoidMethod(jobj, jmid_connecting);
    }
}

void CyPushCallJava::onConnectsuccess() {
    JNIEnv *jniEnv;
    if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
        return;
    }
    jniEnv->CallVoidMethod(jobj, jmid_connectsuccess);
    javaVM->DetachCurrentThread();
}

void CyPushCallJava::onConnectFail(char *msg) {
    JNIEnv *jniEnv;
    if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
        return;
    }

    jstring jmsg = jniEnv->NewStringUTF(msg);

    jniEnv->CallVoidMethod(jobj, jmid_connectfail, jmsg);

    jniEnv->DeleteLocalRef(jmsg);
    javaVM->DetachCurrentThread();
}