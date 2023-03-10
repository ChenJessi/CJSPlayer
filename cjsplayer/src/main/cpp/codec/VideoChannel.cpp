//
// Created by 哦 on 2023/2/15.
//

#include "VideoChannel.h"


/**
 * AVFrame 丢包处理
 * @param queue
 */
void dropAVFrame(queue<AVFrame*>& queue){
    if(!queue.empty()){
        AVFrame *frame = queue.front();
        BaseChannel::releaseAVFrame(&frame);
        queue.pop();
    }
}

/**
 * AVPacket 丢包处理 I 帧不能丢弃
 * @param queue
 */
void dropAVPacket(queue<AVPacket*>& queue){
    while (!queue.empty()){
        AVPacket* packet = queue.front();
        if(packet->flags != AV_PKT_FLAG_KEY){
            // 非关键帧
            BaseChannel::releaseAVPacket(&packet);
            queue.pop();
        }
        else {
            break;
        }
    }
}


VideoChannel::VideoChannel(int stream_index, AVCodecContext *codecContext, AVRational time_base,
                           int fps) : BaseChannel(
        stream_index, codecContext, time_base), fps(fps) {

    frames.setSyncCallback(dropAVFrame);
    packets.setSyncCallback(dropAVPacket);
}

VideoChannel::~VideoChannel() {

}


void *task_video_decode(void *args) {
    auto channel = static_cast<VideoChannel *>(args);
    channel->video_decode();
    return nullptr;
}

void *task_video_play(void *args) {
    auto channel = static_cast<VideoChannel *>(args);
    channel->video_play();
    return nullptr;
}


void VideoChannel::start() {

    isPlaying = true;
    // 缓冲队列开始工作
    packets.setWork(1);
    frames.setWork(1);
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

    while (isPlaying) {
        // 控制 frames 队列大小 控制内存
        if (isPlaying && frames.size() > 100) {
            av_usleep(10 * 1000);
            continue;
        }

        int ret = packets.getQueueAndDel(packet);
        if (!isPlaying) {
            break;
        }
        // 没读取懂啊数据
        if (!ret) {
            continue;
        }

        // 将数据包发送到缓冲区，再从缓冲区获取到原始包
        ret = avcodec_send_packet(codecContext, packet);

        if (ret) {
            // 数据包发送失败，直接结束循环
            break;
        }

        // 从ffmpeg 数据缓冲区获取原始包
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(codecContext, frame);
        if (ret == AVERROR(EAGAIN)) {
            // B帧，参考前面的帧成功，参考后面的帧失败，继续读取下一帧
            continue;
        } else if (ret != 0) {
            // 出现错误
            if (frame) {
                av_frame_unref(frame);
                releaseAVFrame(&frame);
            }
            break;
        }

        frames.insertToQueue(frame);

        av_packet_unref(packet);
        releaseAVPacket(&packet);
    }
    if (packet) {
        av_packet_unref(packet);
        releaseAVPacket(&packet);
    }

}

// 从缓冲队列获取到原始数据包（AVFrame*）进行播放
void VideoChannel::video_play() {
    LOGD("video_play")
    AVFrame *frame = nullptr;
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


    while (isPlaying) {
        int ret = frames.getQueueAndDel(frame);

        if (!isPlaying) {
            // 停止播放
            break;
        }

        if (!ret) {
            // 没获取到数据，继续等待
            continue;
        }

        // 将获取到到原始数据 YUV 格式转为 RGBA
        sws_scale(sws_ctx,
                //输入 yuv 数据
                  frame->data, // 每行的数据
                  frame->linesize,    // 行大小
                  0, frame->height,

                // 输出 数据
                  dst_data,
                  dst_linesize
        );

        // 音视频同步逻辑
        // 视频帧额外的延迟（视频编码到时候选择可以加入一些额外的延迟）
        double extra_delay = frame->repeat_pict / (2 * fps);
        // 根据fps 计算每一帧延迟时间
        double fps_delay = 1.0/fps;
        // 当前帧 真实的延迟时间
        double real_delay = extra_delay + fps_delay;

        // 比较视频和音频的时间戳
        double video_time = frame->best_effort_timestamp * av_q2d(time_base);
        double audio_time = audioChannel->audio_time;
        double time_diff = video_time - audio_time;
        LOGD("video_play time_diff %lf", time_diff)
        if(time_diff > 0){
            // 视频时间 > 音频时间，视频需要等待
            if(time_diff > 1){
                // 视频和音频差距太大，卡太久体验很不好
                // 稍微延迟两帧的间隔即可
                av_usleep(((real_delay * 2) * 1000000));
            } else {
                // 差距不大的情况下，差多少延迟多少即可 （当前帧的延迟时间+音视频时间差）
                av_usleep(((real_delay + time_diff) *1000000));
            }
        } else if(time_diff > 0){
            // 视频时间 < 音频时间， 视频要播放快一点 ，丢包处理
            // 注意 I 帧不能丢， 需要丢掉 packets 和 frames 的数据包
            // 经验值 0.05 以内不需要丢包
            if(fabs(time_diff) >= 0.05){
                packets.sync();
                frames.sync();
                continue;
            }
        } else{
            // 音视频完全同步
        }



        // 渲染
        //  将数据回调出去
        renderCallback(dst_data[0], codecContext->width, codecContext->height, dst_linesize[0]);
        // 使用完之后要释放
        av_frame_unref(frame);
        releaseAVFrame(&frame);
    }
    // 释放
    if (frame) {
        av_frame_unref(frame);
        releaseAVFrame(&frame);
    }

    isPlaying = false;
    av_free(&dst_data[0]);
    sws_freeContext(sws_ctx);

}

void VideoChannel::setRenderCallback(RenderCallback callback) {
    this->renderCallback = callback;
}

void VideoChannel::setAudioChannel(AudioChannel *audio_channel) {
    this->audioChannel = audio_channel;
}

