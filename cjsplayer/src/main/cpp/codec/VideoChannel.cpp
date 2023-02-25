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

void* task_video_play(void * args){
    auto channel = static_cast<VideoChannel *>(args);
    channel->video_play();
    return nullptr;
}



void VideoChannel::start() {

    isPlaying = true;

    // 视频解码线程：取出数据队列的压缩包，解码之后放回原始数据包队列
    pthread_create(&pid_video_decode, nullptr, task_video_decode, this);
    // 播放线程：从原始数据包中读取数据 并进行格式转化，播放
    pthread_create(&pid_video_play, nullptr, task_video_play, this);
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
            // 出现错误
            break;
        }

        frames.insertToQueue(frame);
    }
    releaseAVPacket(&packet);
}

// 从缓冲队列获取到原始数据包（AVFrame*）进行播放
void VideoChannel::video_play() {

    AVFrame* avFrame = nullptr;
    uint8_t *dst_data[4]; // ARGB 4位
    int dst_linesize[4]; //ARGB

    // ffmpeg 原始数据 AVFrame 是 YUV 格式数据，Android屏幕是ARGB格式，需要转化

    //给 dst_data 申请内存
    av_image_alloc(dst_data, dst_linesize,
                   codecContext->width, codecContext->height, AV_PIX_FMT_RGBA, 1);

    SwsContext *sws_ctx = sws_getContext(
            // 输入信息
            codecContext->width,
            codecContext->height,
            codecContext->pix_fmt, // 获取 mp4 视频的像素格式 AV_PIX_FMT_YUV420P

            // 输出信息 宽，高，格式
            codecContext->width,
            codecContext->height,
            AV_PIX_FMT_RGBA,
            SWS_BILINEAR, // 转换算法，选 SWS_BILINEAR适中一点的
            nullptr, nullptr, nullptr);


    while (isPlaying){
        int ret = frames.getQueueAndDel(avFrame);

        if(!isPlaying){
            // 停止播放
            break;
        }

        if(!ret){
            // 没获取到数据，继续等待
            continue;
        }

        // 将获取到到原始数据 YUV 格式转为 RGBA
        sws_scale(sws_ctx,
                //输入 yuv 数据
                  avFrame->data, // 每行的数据
                  avFrame->linesize,    // 行大小
                  0,  avFrame->height,

                // 输出 数据
                  dst_data,
                  dst_linesize
                  );


        // 渲染
        //  将数据回调出去
        renderCallback(dst_data[0], codecContext->width, codecContext->height, dst_linesize[0]);
        // 使用完之后要释放
        releaseAVFrame(&avFrame);
    }
    // 释放
    releaseAVFrame(&avFrame);
    isPlaying = false;
    av_free(&dst_data[0]);
    sws_freeContext(sws_ctx);

}

void VideoChannel::setRenderCallback(RenderCallback callback) {
    this->renderCallback = callback;
}

