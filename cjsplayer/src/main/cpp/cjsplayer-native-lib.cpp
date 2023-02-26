//
// Created by 哦 on 2023/2/11.
//


#include <jni.h>
#include "player/CJSPlayer.h"
#include "utils/JNICallbackHelper.h"
#include "pthread.h"
#include "android/native_window_jni.h"





JavaVM *vm = nullptr;
jint JNI_OnLoad(JavaVM * vm, void * args){
    ::vm = vm;
    return JNI_VERSION_1_6;
}
CJSPlayer *player = nullptr;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
ANativeWindow *window = nullptr;

/**
 * 进行渲染工作
 */
void renderFrame(uint8_t* src_data, int width, int height, int src_lineSize){
    pthread_mutex_lock(&mutex);

    if (!window){
        pthread_mutex_unlock(&mutex);
    }

    // 设置窗口属性大小等
    ANativeWindow_setBuffersGeometry(window, width, height, WINDOW_FORMAT_RGBA_8888);

    // 缓冲区
    ANativeWindow_Buffer window_buffer;

    // 如果正在渲染，就要先释放
    if(ANativeWindow_lock(window, &window_buffer, nullptr)){
        ANativeWindow_release(window);
        window = nullptr;
        pthread_mutex_unlock(&mutex);
        return;
    }

    // 开始渲染
    auto dst_data = static_cast<uint8_t *>(window_buffer.bits);
    
    int dst_linesize = window_buffer.stride;
    for (int i = 0; i < window_buffer.height; ++i) {
        //填充数据, 将 视频数据 拷贝到 ANativeWindow  每次填充一行
        // ANativeWindow_Buffer 采用64字节对齐算法，所以 内存大小需要被64位整除，否则会花屏
        // FFmpeg 默认采用了8字节对齐的算法
        // ANativeWindow_Buffer 内部会有隐性的占位值，所以 dst_linesize 大小笔实际数据要大
        memcpy(dst_data + i*dst_linesize, src_data+i*src_lineSize, dst_linesize);
    }
    // 刷新 window
    ANativeWindow_unlockAndPost(window);
    pthread_mutex_unlock(&mutex);
}



extern "C"
JNIEXPORT void JNICALL
Java_com_jessi_cjsplayer_player_manager_CJSPlayerManager_prepareNative(JNIEnv *env, jobject job,
                                                                       jstring source) {
    const char *data_source = env->GetStringUTFChars(source,0);

    auto *helper = new JNICallbackHelper(vm, env, job);
    player = new CJSPlayer(data_source, helper);
    player->prepare();
    player->setRenderCallback(renderFrame);
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

}
extern "C"
JNIEXPORT void JNICALL
Java_com_jessi_cjsplayer_player_manager_CJSPlayerManager_releaseNative(JNIEnv *env, jobject thiz) {

}


extern "C"
JNIEXPORT void JNICALL
Java_com_jessi_cjsplayer_player_manager_CJSPlayerManager_setSurfaceNative(JNIEnv *env, jobject thiz,
                                                                          jobject surface) {
    pthread_mutex_lock(&mutex);
    if(window){
        // 先释放之前的window
        ANativeWindow_release(window);
        window = nullptr;
    }

    window = ANativeWindow_fromSurface(env, surface);
    pthread_mutex_unlock(&mutex);
}