//
// Created by 哦 on 2023/4/9.
//

#ifndef CJSPLAYER_VIDEOPUSHCHANNEL_H
#define CJSPLAYER_VIDEOPUSHCHANNEL_H

#include <rtmp.h>
#include <x264.h>

class VideoPushChannel {

public:
    typedef void (*VideoCallback)(RTMPPacket *packet);




public:
    VideoPushChannel();

    ~VideoPushChannel();

    void setVideoCallback(VideoCallback callback);
    // 初始化编码器
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
    x264_t *videoCodec = nullptr;
    x264_picture_t *pic_in = nullptr;
    VideoCallback videoCallback;
};




#endif //CJSPLAYER_VIDEOPUSHCHANNEL_H
