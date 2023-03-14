//
// Created by 哦 on 2023/2/21.
//

#ifndef CJSPLAYER_BASECHANNEL_H
#define CJSPLAYER_BASECHANNEL_H

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavutil/time.h>
};

#include "../utils/JNICallbackHelper.h"
#include "../queue/safe_queue.h"


class BaseChannel {


public:

    BaseChannel(int stream_index, AVCodecContext *codecContext,AVRational time_base)
        : stream_index(stream_index),
          codecContext(codecContext),
          time_base(time_base){
        packets.setReleaseCallback(releaseAVPacket);
        frames.setReleaseCallback(releaseAVFrame);
    };
    ~BaseChannel(){
        packets.clear();
        frames.clear();
    };


    // 音视频流的位置
    int stream_index = 0;
    // 数据压缩包
    SafeQueue<AVPacket*> packets;
    // 数据原始包
    SafeQueue<AVFrame*> frames;
    bool isPlaying = false;
    AVCodecContext *codecContext = nullptr;

    // 时间基 音视频时间单位，用来做音视频同步
    // 以音频作为主时钟
    AVRational time_base;
    static void releaseAVPacket(AVPacket **packet){
        if(packet){
            av_packet_free(packet);
            *packet = nullptr;
        }
    }

    static void releaseAVFrame(AVFrame **frame){
        if(frame){
            av_frame_free(frame);
            *frame = nullptr;
        }
    }

    JNICallbackHelper *jniCallbackHelper = nullptr;
    void setJNICallbackHelper(JNICallbackHelper *helper){
        jniCallbackHelper = helper;
    }
};


#endif //CJSPLAYER_BASECHANNEL_H
