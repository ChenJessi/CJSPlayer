//
// Created by CHEN on 2019/1/15.
//

#ifndef FFMPEGPLAYER_ANDROIDLOG_H
#define FFMPEGPLAYER_ANDROIDLOG_H

#include <android/log.h>

#define LOG_SHOW false;

#define  LOGD(FORMAT,...) __android_log_print(ANDROID_LOG_DEBUG,"ffmpeg",FORMAT,##__VA_ARGS__);
#define  LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"ffmpeg",FORMAT,##__VA_ARGS__);
#define  LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"ffmpeg",FORMAT,##__VA_ARGS__);

#endif //FFMPEGPLAYER_ANDROIDLOG_H