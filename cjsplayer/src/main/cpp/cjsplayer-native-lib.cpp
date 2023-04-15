//
// Created by 哦 on 2023/2/11.
//


#include <jni.h>
#include "player/CJSPlayer.h"
#include "utils/JNICallbackHelper.h"
#include "pthread.h"
#include "android/native_window_jni.h"

#include "livepush/VideoPushChannel.h"



JavaVM *vm = nullptr;
jint JNI_OnLoad(JavaVM * vm, void * args){
    ::vm = vm;
    return JNI_VERSION_1_6;
}

/******************************* player  start *******************************************************/

//<editor-fold desc="Player ">

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
    // rgba8888 32位4字节
    int dst_linesize = window_buffer.stride * 4;
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
Java_com_jessi_cjsplayer_manager_CJSPlayerManager_prepareNative(JNIEnv *env, jobject job,
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
Java_com_jessi_cjsplayer_manager_CJSPlayerManager_startNative(JNIEnv *env, jobject thiz) {
    if(player){
        player->start();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_jessi_cjsplayer_manager_CJSPlayerManager_stopNative(JNIEnv *env, jobject thiz) {

    if(player){
        player->stop();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_jessi_cjsplayer_manager_CJSPlayerManager_releaseNative(JNIEnv *env, jobject thiz) {

    LOGE("releaseNative")
    pthread_mutex_lock(&mutex);
    LOGE("releaseNative mutex")
    if(window){
        ANativeWindow_release(window);
        window = nullptr;
    }
    pthread_mutex_unlock(&mutex);

    if(player){
        delete player;
        player = nullptr;
    }

    DELETE(vm)
    DELETE(window)
}


extern "C"
JNIEXPORT void JNICALL
Java_com_jessi_cjsplayer_manager_CJSPlayerManager_setSurfaceNative(JNIEnv *env, jobject thiz,
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


extern "C"
JNIEXPORT jint JNICALL
Java_com_jessi_cjsplayer_manager_CJSPlayerManager_getDurationNative(JNIEnv *env, jobject thiz) {

    if(player){
        return player->getDuration();
    }
    return 0;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_jessi_cjsplayer_manager_CJSPlayerManager_seekNative(JNIEnv *env, jobject thiz, jint secs) {
    if(player){
        player->seek(secs);
    }
}
//</editor-fold>
/******************************* player  end *******************************************************/

/******************************* push  start *******************************************************/

//<editor-fold desc=" Push">



VideoPushChannel *videoPushChannel = nullptr;
SafeQueue<RTMPPacket *> packets;    // 保存打包后的音视频数据
bool isStartPush = false;
pthread_t pid_start;
bool readyPushing = false;
uint32_t startTime;

/**
 * 将打包后的视频数据保存到队列
 * @param packet
 */
void videoCallback(RTMPPacket *packet) {
    if (packet) {
        if (packet->m_nTimeStamp == -1){ // -1 说明是数据帧
            packet->m_nTimeStamp = RTMP_GetTime() - startTime;
        }
        packets.insertToQueue(packet);
    }
}

void releasePacket(RTMPPacket **packet) {
    if(packet){
        RTMPPacket_Free(*packet);
        delete packet;
        packet = nullptr;
    }
}

void *task_start_push(void *args) {
    char *url = static_cast<char *>(args);

    int ret = 0;
    // RTMP 初始化
    RTMP *rtmp = nullptr;
    rtmp = RTMP_Alloc();
    do{
        if (!rtmp) {
            LOGE("RTMP_Alloc failed");
            break;
        }
        // 初始化
        RTMP_Init(rtmp);
        // 设置超时时间
        rtmp->Link.timeout = 5;
        // 设置推流地址
        ret = RTMP_SetupURL(rtmp, url);
        if (!ret) {
            LOGE("RTMP_SetupURL failed");
            break;
        }
        // 设置可写
        RTMP_EnableWrite(rtmp);

        // 连接服务器
        ret = RTMP_Connect(rtmp, nullptr);
        if (!ret) {
            LOGE("RTMP_Connect failed %d url: %s", ret, url);
            break;
        }

        // 连接流
        ret = RTMP_ConnectStream(rtmp, 0);
        if (!ret) {
            LOGE("RTMP_ConnectStream failed");
            break;
        }

        startTime = RTMP_GetTime();
        // 开始推流
        readyPushing = true;
        // 从队列里面获取数据包，发送给服务器

        packets.setWork(1);
        RTMPPacket *packet = nullptr;

        while (readyPushing){

            packets.getQueueAndDel(packet);
            if(!readyPushing){
                break;
            }
            if(!packet){
                continue;
            }
            // 成功取出队列中的数据，直接发送给服务器

            // 给数据流设置id
            packet->m_nInfoField2 = rtmp->m_stream_id;
            // 发送数据包
            ret = RTMP_SendPacket(rtmp, packet, 1);

            // 释放内存
            releasePacket(&packet);

            if(!ret){
                LOGE("RTMP_SendPacket failed");
                break;
            }
            LOGE("RTMP_SendPacket success")
        }
        releasePacket(&packet);
    }while (false);

    // 释放资源
    isStartPush = false;
    readyPushing = false;
    packets.setWork(0);
    packets.clear();
    if (rtmp) {
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
    }

    delete url;
    return nullptr;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_jessi_cjsplayer_push_CJSPusher_initNative(JNIEnv *env, jobject thiz) {

    videoPushChannel = new VideoPushChannel();
    videoPushChannel->setVideoCallback(videoCallback);
    packets.setReleaseCallback(releasePacket);

}

extern "C"
JNIEXPORT void JNICALL
Java_com_jessi_cjsplayer_push_CJSPusher_startLiveNative(JNIEnv *env, jobject thiz, jstring path_) {
    if (isStartPush) {
        return;
    }
    isStartPush = true;
    const char *data_url = const_cast<char *>(env->GetStringUTFChars(path_, nullptr));
    char *url = new char[strlen(data_url) + 1];
    strcpy(url, data_url);

    pthread_create(&pid_start, nullptr, task_start_push, url);

    env->ReleaseStringUTFChars(path_, data_url);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_jessi_cjsplayer_push_CJSPusher_stopLiveNative(JNIEnv *env, jobject thiz) {

}
extern "C"
JNIEXPORT void JNICALL
Java_com_jessi_cjsplayer_push_CJSPusher_releaseNative(JNIEnv *env, jobject thiz) {

}

extern "C"
JNIEXPORT void JNICALL
Java_com_jessi_cjsplayer_push_CJSPusher_initVideoEncoderNative(JNIEnv *env, jobject thiz,
                                                               jint width, jint height, jint fps,
                                                               jint bitrate) {
    if (videoPushChannel) {
        videoPushChannel->initVideoEncoder(width, height, fps, bitrate);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jessi_cjsplayer_push_CJSPusher_pushVideoNative(JNIEnv *env, jobject thiz,
                                                        jbyteArray data) {
    LOGE("pushVideoNative ")
    if (!videoPushChannel || !readyPushing) {
        return;
    }
    // 相机的n21数据
    jbyte *video_data = env->GetByteArrayElements(data, nullptr);
    if (videoPushChannel) {
        videoPushChannel->encodeData(video_data);
    }
    env->ReleaseByteArrayElements(data, video_data, 0);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_jessi_cjsplayer_push_CJSPusher_initAudioEncoderNative(JNIEnv *env, jobject thiz,
                                                               jint sample_rate_in_hz,
                                                               jint channel) {

}


extern "C"
JNIEXPORT jint JNICALL
Java_com_jessi_cjsplayer_push_CJSPusher_getInputSamplesNative(JNIEnv *env, jobject thiz) {

    return 4096;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_jessi_cjsplayer_push_CJSPusher_pushAudioNative(JNIEnv *env, jobject thiz,
                                                        jbyteArray data) {

}

//</editor-fold>
/******************************* push  end *******************************************************/


