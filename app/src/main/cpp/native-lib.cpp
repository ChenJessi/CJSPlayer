#include <jni.h>
#include <string>
#include "androidLog.h"
#include "CyCallJava.h"
#include "CyFFmpeg.h"

extern "C"
{
#include <libavformat/avformat.h>
#import <SLES/OpenSLES.h>
#import <SLES/OpenSLES_Android.h>
}


_JavaVM *javaVM = NULL;
CyCallJava *callJava = NULL;
CyFFmpeg *fFmpeg = NULL;
CyPlaystatus *playstatus = NULL;
bool nexit = true;
pthread_t thread_start;


extern "C"
JNIEXPORT void JNICALL
Java_com_chen_cyplayer_player_CyPlayer_n_1prepared(JNIEnv *env, jobject instance, jstring source_) {
    const char *source = env->GetStringUTFChars(source_, 0);

    if (fFmpeg == NULL){
        if (callJava == NULL){
            callJava = new CyCallJava(javaVM, env, &instance);
        }
        callJava->onCallLoad(MAIN_THREAD, true);
        playstatus = new CyPlaystatus();
        fFmpeg = new CyFFmpeg(playstatus,callJava,source);
        fFmpeg->prepared();
    }
}

void *startCallBack(void *data){
    CyFFmpeg *cyFFmpeg = (CyFFmpeg *)data;
    cyFFmpeg->start();
    return 0;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_chen_cyplayer_player_CyPlayer_n_1start(JNIEnv *env, jobject instance) {
    // TODO
    if (fFmpeg != NULL){
        pthread_create(&thread_start, NULL, startCallBack , fFmpeg);
    }
}

extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved){
    jint result = -1;
    javaVM = vm;
    JNIEnv *env;
    if (vm->GetEnv((void **)(&env), JNI_VERSION_1_4) != JNI_OK){
        return result;
    }
    return JNI_VERSION_1_4;
};

extern "C"
JNIEXPORT void JNICALL
Java_com_chen_cyplayer_player_CyPlayer_n_1pause(JNIEnv *env, jobject instance) {

    // TODO
    if (fFmpeg != NULL){
        fFmpeg->pause();
    }

}extern "C"
JNIEXPORT void JNICALL
Java_com_chen_cyplayer_player_CyPlayer_n_1resume(JNIEnv *env, jobject instance) {
    // TODO
    if (fFmpeg != NULL){
        fFmpeg->resume();
    }

}extern "C"
JNIEXPORT void JNICALL
Java_com_chen_cyplayer_player_CyPlayer_n_1stop(JNIEnv *env, jobject instance) {
    // TODO
    if (!nexit){
        return;
    }
    jclass clz = env->GetObjectClass(instance);
    jmethodID jmid_next = env->GetMethodID(clz, "onCallNext","()V");
    nexit = false;
    if (fFmpeg != NULL){
        fFmpeg->release();
        pthread_join(thread_start, NULL);
        delete(fFmpeg);
        LOGD("fFmpeg 释放")
        fFmpeg = NULL;
        if (callJava != NULL){
            delete(callJava);
            callJava = NULL;
            LOGD("callJava 释放")
        }
        if (playstatus != NULL){
            delete(playstatus);
            playstatus = NULL;
            LOGD("playstatus 释放")
        }
    }
    nexit = true;
    env->CallVoidMethod(instance, jmid_next);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_chen_cyplayer_player_CyPlayer_n_1seek(JNIEnv *env, jobject instance, jint secds) {

    // TODO
    if (fFmpeg != NULL){
        LOGE("seek 1time %d", secds);
        fFmpeg->seek(secds);
    }

}

extern "C"
JNIEXPORT jint JNICALL
Java_com_chen_cyplayer_player_CyPlayer_n_1duration(JNIEnv *env, jobject instance) {
    // TODO
    if (fFmpeg != NULL){
        return fFmpeg->duration;
    }
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_chen_cyplayer_player_CyPlayer_n_1volume(JNIEnv *env, jobject instance, jint percent) {

    // TODO
    if (fFmpeg != NULL){
        fFmpeg->setVolume(percent);
    }
}extern "C"
JNIEXPORT void JNICALL
Java_com_chen_cyplayer_player_CyPlayer_n_1mute(JNIEnv *env, jobject instance, jint mute) {
    // TODO
    if (fFmpeg != NULL){
        fFmpeg->setMute(mute);
    }

}extern "C"
JNIEXPORT void JNICALL
Java_com_chen_cyplayer_player_CyPlayer_n_1pitch(JNIEnv *env, jobject instance, jfloat pitch) {
    // TODO
    if (fFmpeg != NULL){
        fFmpeg->setPitch(pitch);
    }
}extern "C"
JNIEXPORT void JNICALL
Java_com_chen_cyplayer_player_CyPlayer_n_1speed(JNIEnv *env, jobject instance, jfloat speed) {
    // TODO
    if (fFmpeg != NULL){
        fFmpeg->setSpeed(speed);
    }
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_chen_cyplayer_player_CyPlayer_n_1samplerate(JNIEnv *env, jobject instance) {
    // TODO
    if (fFmpeg != NULL){
        return  fFmpeg->getSampleRate();
    }
    return  0;
}extern "C"
JNIEXPORT void JNICALL
Java_com_chen_cyplayer_player_CyPlayer_n_1startstoprecord(JNIEnv *env, jobject instance,
                                                          jboolean start) {
    // TODO
    if (fFmpeg != NULL){
        fFmpeg->startStopRecord(start);
    }
}extern "C"
JNIEXPORT jboolean JNICALL
Java_com_chen_cyplayer_player_CyPlayer_n_1cutaudioplay(JNIEnv *env, jobject instance,
                                                       jint start_time, jint end_time,
                                                       jboolean showPcm) {
    // TODO
    if (fFmpeg != NULL){
        return fFmpeg->cutAudioPlay(start_time, end_time, showPcm);
    }
    return false;
}