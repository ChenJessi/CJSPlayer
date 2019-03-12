//
// Created by CHEN on 2019/2/20.
//

#ifndef CYPLAYER_ANDROIDLOG_H
#define CYPLAYER_ANDROIDLOG_H

#endif //CYPLAYER_ANDROIDLOG_H

#include <android/log.h>
#define LOG_DEBUG true
#define  LOGD(FORMAT,...) __android_log_print(ANDROID_LOG_DEBUG,"cyvideo",FORMAT,##__VA_ARGS__);
#define  LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"cyvideo",FORMAT,##__VA_ARGS__);
#define  LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"cyvideo",FORMAT,##__VA_ARGS__);