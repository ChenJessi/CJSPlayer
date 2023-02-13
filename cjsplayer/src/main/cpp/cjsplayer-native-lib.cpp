#include <jni.h>
#include "player/CJSPlayer.h"

//
// Created by 哦 on 2023/2/11.
//

extern "C"
JNIEXPORT void JNICALL
Java_com_jessi_cjsplayer_player_manager_CJSPlayerManager_prepareNative(JNIEnv *env, jobject thiz,
                                                                       jstring source) {
    const char *data_source = env->GetStringUTFChars(source,0);

    auto player = new CJSPlayer(data_source);
    env->ReleaseStringUTFChars(source, data_source);

}




extern "C"
JNIEXPORT void JNICALL
Java_com_jessi_cjsplayer_player_manager_CJSPlayerManager_startNative(JNIEnv *env, jobject thiz) {
    // TODO: implement startNative()
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