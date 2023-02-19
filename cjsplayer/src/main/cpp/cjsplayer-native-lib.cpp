#include <jni.h>
#include "player/CJSPlayer.h"
#include "utils/JNICallbackHelper.h"
//
// Created by 哦 on 2023/2/11.
//


JavaVM *vm = nullptr;
jint JNI_OnLoad(JavaVM * vm, void * args){
    ::vm = vm;
    return JNI_VERSION_1_6;
}
CJSPlayer *player = nullptr;


extern "C"
JNIEXPORT void JNICALL
Java_com_jessi_cjsplayer_player_manager_CJSPlayerManager_prepareNative(JNIEnv *env, jobject job,
                                                                       jstring source) {
    const char *data_source = env->GetStringUTFChars(source,0);

    auto *helper = new JNICallbackHelper(vm, env, job);
    player = new CJSPlayer(data_source, helper);
    player->prepare();
    env->ReleaseStringUTFChars(source, data_source);

}

extern "C"
JNIEXPORT void JNICALL
Java_com_jessi_cjsplayer_player_manager_CJSPlayerManager_startNative(JNIEnv *env, jobject thiz) {
    if(player){
        player->start();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_jessi_cjsplayer_player_manager_CJSPlayerManager_stopNative(JNIEnv *env, jobject thiz) {
    // TODO: implement stopNative()
}
extern "C"
JNIEXPORT void JNICALL
Java_com_jessi_cjsplayer_player_manager_CJSPlayerManager_releaseNative(JNIEnv *env, jobject thiz) {
    // TODO: implement releaseNative()
}