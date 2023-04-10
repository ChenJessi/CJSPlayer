//
// Created by 哦 on 2023/4/9.
//

#ifndef CJSPLAYER_VIDEOPUSHCHANNEL_H
#define CJSPLAYER_VIDEOPUSHCHANNEL_H

#include <rtmp.h>
#include <x264.h>
#include <pthread.h>
#include "../utils/AndroidLog.hpp"
#include <cstring>

class VideoPushChannel {

public:
    typedef void (*VideoCallback)(RTMPPacket *packet);




public:
    VideoPushChannel();

    ~VideoPushChannel();

    void setVideoCallback(VideoCallback callback);
    // 初始化编码器 x264
    void initVideoEncoder(int width, int height, int fps, int bitrate);
    // 对相机数据进行编码
    void encodeData(signed char *data);
private:
    int mFps; // 帧率
    int mBitrate; // 码率
    int mWidth; // 宽
    int mHeight; // 高
    int y_len; // y的长度
    int uv_len; // uv的长度
    x264_t *videoCodec = nullptr; // 编码器
    x264_picture_t *pic_in = nullptr; // 输入的图像
    VideoCallback videoCallback;
    pthread_mutex_t mutex;

    void sendSpsPps(uint8_t sps[100], int sps_len, uint8_t pps[100], int pps_len);

    void sendFrame(int type, uint8_t *pPayload, int payload);
};




#endif //CJSPLAYER_VIDEOPUSHCHANNEL_H
