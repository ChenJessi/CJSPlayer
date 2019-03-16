//
// Created by CHEN on 2019/2/26.
//

#include "CyCallJava.h"
#include "androidLog.h"

CyCallJava::CyCallJava(_JavaVM *javaVM, JNIEnv *env, jobject *obj) {

    this->javaVM = javaVM;
    this->jniEnv = env;
    this->jobj = *obj;
    this->jobj = env->NewGlobalRef(jobj);

    jclass jlz = jniEnv->GetObjectClass(jobj);
    if (!jlz){
        if (LOG_DEBUG){
            LOGE("get jclass wrong!");
        }
        return ;
    }
    jmid_prepared = env->GetMethodID(jlz,"onCallPrepared","()V");
    jmid_load = env->GetMethodID(jlz,"onCallLoad","(Z)V");
    jmid_timeInfo = env->GetMethodID(jlz,"onCallTimeInfo","(II)V");
    jmid_error = env->GetMethodID(jlz,"onCallError","(ILjava/lang/String;)V");
    jmid_complete = env->GetMethodID(jlz,"onCallComplete","()V");
    jmid_valumeDB = env->GetMethodID(jlz,"onCallValumeDB","(I)V");
    jmid_pcmtoaac = env->GetMethodID(jlz, "encodecPcmToAAc", "(I[B)V");
    jmid_pcminfo = env->GetMethodID(jlz, "onCallPcmInfo", "(I[B)V");
    jmid_pcmrate = env->GetMethodID(jlz, "onCallPcmRate", "(I)V");
}

void CyCallJava::onCallPrepared(int type) {
    if (type == MAIN_THREAD){
        jniEnv->CallVoidMethod(jobj,jmid_prepared);
    } else if (type == CHILD_THREAD){
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv,0) != JNI_OK){
            if (LOG_DEBUG) {
                LOGE("get child thread jnienv worng!");
            }
            return;
        }
        jniEnv->CallVoidMethod(jobj,jmid_prepared);
        javaVM->DetachCurrentThread();
    };
}

void CyCallJava::onCallLoad(int type, bool load) {
    if (type == MAIN_THREAD){
        jniEnv->CallVoidMethod(jobj,jmid_load);
    } else if (type == CHILD_THREAD){
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv,0) != JNI_OK){
            if (LOG_DEBUG) {
                LOGE("call onCallLoad worng!");
            }
            return;
        }
        jniEnv->CallVoidMethod(jobj,jmid_load,load);
        javaVM->DetachCurrentThread();
    };
}

void CyCallJava::onCallTimeInfo(int type, int currentTime, int totalTime) {
    if (type == MAIN_THREAD){
        jniEnv->CallVoidMethod(jobj,jmid_timeInfo);
    } else if (type == CHILD_THREAD){
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv,0) != JNI_OK){
            if (LOG_DEBUG) {
                LOGE("call onCallTimeInfo worng!");
            }
            return;
        }
        jniEnv->CallVoidMethod(jobj,jmid_timeInfo, currentTime, totalTime);
        javaVM->DetachCurrentThread();
    };
}

CyCallJava::~CyCallJava() {

}

void CyCallJava::onCallError(int type, int code, char *msg) {
    if (type == MAIN_THREAD){
        jstring jmsg = jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(jobj, jmid_error, code, jmsg);
        jniEnv->DeleteLocalRef(jmsg);
    } else if (type == CHILD_THREAD){
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv,0) != JNI_OK){
            if (LOG_DEBUG) {
                LOGE("call onCallError worng!");
            }
            return;
        }
        jstring jmsg = jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(jobj, jmid_error, code, jmsg);
        jniEnv->DeleteLocalRef(jmsg);
        javaVM->DetachCurrentThread();
    };

}

void CyCallJava::onCallComplete(int type) {
    if (type == MAIN_THREAD){
        jniEnv->CallVoidMethod(jobj,jmid_complete);
    } else if (type == CHILD_THREAD){
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv,0) != JNI_OK){
            if (LOG_DEBUG) {
                LOGE("call onCallComplete worng!");
            }
            return;
        }
        jniEnv->CallVoidMethod(jobj,jmid_complete);
        javaVM->DetachCurrentThread();
    };
}

void CyCallJava::onCallValumeDB(int type, int db) {
    if (type == MAIN_THREAD){
        jniEnv->CallVoidMethod(jobj,jmid_valumeDB);
    } else if (type == CHILD_THREAD){
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv,0) != JNI_OK){
            if (LOG_DEBUG) {
                LOGE("call onCallValumeDB worng!");
            }
            return;
        }
        jniEnv->CallVoidMethod(jobj,jmid_valumeDB, db);
        javaVM->DetachCurrentThread();
    };
}

void CyCallJava::onCallPcmToAAC(int type, int size, void *buffer) {
    if(type == MAIN_THREAD) {
        jbyteArray jbuffer = jniEnv->NewByteArray(size);
        jniEnv->SetByteArrayRegion(jbuffer, 0, size, static_cast<const jbyte *>(buffer));

        jniEnv->CallVoidMethod(jobj, jmid_pcmtoaac, size, jbuffer);

        jniEnv->DeleteLocalRef(jbuffer);

    }
    else if(type == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if(javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            if(LOG_DEBUG) {
                LOGE("call onCallPcmToAAC worng");
            }
            return;
        }
        jbyteArray jbuffer = jniEnv->NewByteArray(size);
        jniEnv->SetByteArrayRegion(jbuffer, 0, size, static_cast<const jbyte *>(buffer));

        jniEnv->CallVoidMethod(jobj, jmid_pcmtoaac, size, jbuffer);

        jniEnv->DeleteLocalRef(jbuffer);

        javaVM->DetachCurrentThread();
    }
}

void CyCallJava::onCallPcmInfo(int size, void *buffer) {
    JNIEnv *jniEnv;
    if(javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
        if(LOG_DEBUG) {
            LOGE("call onCallPcmInfo worng");
        }
        return;
    }
    jbyteArray jbuffer = jniEnv->NewByteArray(size);
    jniEnv->SetByteArrayRegion(jbuffer, 0, size, static_cast<const jbyte *>(buffer));

    jniEnv->CallVoidMethod(jobj, jmid_pcminfo, size, jbuffer);

    jniEnv->DeleteLocalRef(jbuffer);

    javaVM->DetachCurrentThread();
}

void CyCallJava::onCallPcmRate(int samplerate) {
    JNIEnv *jniEnv;
    if(javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK)
    {
        if(LOG_DEBUG)
        {
            LOGE("call onCallPcmRate worng");
        }
        return;
    }
    jniEnv->CallVoidMethod(jobj, jmid_pcmrate, samplerate);

    javaVM->DetachCurrentThread();
}

