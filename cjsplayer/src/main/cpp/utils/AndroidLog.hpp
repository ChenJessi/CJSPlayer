//
// Created by 哦 on 2023/2/17.
//

#ifndef CJSPLAYER_ANDROIDLOG_HPP
#define CJSPLAYER_ANDROIDLOG_HPP

#include <android/log.h>
#define LOG_DEBUG true
#define  LOGD(FORMAT,...) __android_log_print(ANDROID_LOG_DEBUG,"cjsplayer",FORMAT,##__VA_ARGS__);
#define  LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"cjsplayer",FORMAT,##__VA_ARGS__);
#define  LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"cjsplayer",FORMAT,##__VA_ARGS__);

#endif //CJSPLAYER_ANDROIDLOG_HPP
