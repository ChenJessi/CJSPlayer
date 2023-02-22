//
// Created by 哦 on 2023/2/15.
//

#include "VideoChannel.h"

VideoChannel::VideoChannel(int stream_index, AVCodecContext *codecContext) : BaseChannel(
        stream_index, codecContext) {

}

VideoChannel::~VideoChannel() {

}



void* task_video_decode(void * args){
    auto channel = static_cast<VideoChannel *>(args);
    channel->video_decode();
    return nullptr;
}



void VideoChannel::start() {

    isPlaying = true;

    // 视频解码线程：取出数据队列的压缩包，解码之后放回原始数据包队列
    pthread_create(&pid_video_decode, nullptr, task_video_decode, this);
}

void VideoChannel::stop() {

}

// 把队列里面的压缩包(AVPacket*)取出来，然后解码成（AVFrame*）原始包
void VideoChannel::video_decode() {
    AVPacket *packet = nullptr;

    while (isPlaying){
        int ret = packets.getQueueAndDel(packet);
        if(!isPlaying){
            break;
        }

        // 没读取懂啊数据
        if(!ret){
            continue;
        }

        // 将数据包发送到缓冲区，再从缓冲区获取到原始包
        ret = avcodec_send_packet(codecContext, packet);

        releaseAVPacket(&packet);

        if(ret){
            // 数据包发送失败，直接结束循环
            break;
        }

        // 从ffmpeg 数据缓冲区获取原始包

        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(codecContext, frame);
        if(ret == AVERROR(EAGAIN)){
            // B帧，参考前面的帧成功，参考后面的帧失败，继续读取下一帧
            continue;
        }
        else if(ret != 0){
            break;
        }

        frames.insertToQueue(frame);
    }
    releaseAVPacket(&packet);
}

