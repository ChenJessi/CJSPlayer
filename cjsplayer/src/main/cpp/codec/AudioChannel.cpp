//
// Created by 哦 on 2023/2/15.
//

#include "AudioChannel.h"
 // channel


AudioChannel::AudioChannel(int stream_index, AVCodecContext *codecContext) : BaseChannel(
        stream_index, codecContext) {

}



AudioChannel::~AudioChannel() {

}




void* task_audio_decode(void * args){
    auto channel = static_cast<AudioChannel*>(args);
    channel->audio_decode();
    return nullptr;
}

void* task_audio_play(void * args){
    auto channel = static_cast<AudioChannel*>(args);
    channel->audio_play();
    return nullptr;
}




void AudioChannel::start() {
    isPlaying = true;

    //缓冲队列开始工作
    packets.setWork(1);
    frames.setWork(1);

    // 音频解码线程
    pthread_create(&pid_audio_decode, nullptr, task_audio_decode, this);
    // 获取音频原始数据，播放
    pthread_create(&pid_audio_play, nullptr, task_audio_play, this);
}

void AudioChannel::stop() {

}
/**
 *  取出数据压缩包，解码为原始包 AVFrame*
 */
void AudioChannel::audio_decode() {
    AVPacket *packet = nullptr;

    while (isPlaying){
        int ret = packets.getQueueAndDel(packet);
        if(!isPlaying){
            break;
        }

        if(!ret){
            // 获取失败之后继续获取
            continue;
        }

        // 将压缩包发送到缓冲区，再从缓冲区获取到原始包
        ret = avcodec_send_packet(codecContext, packet);

        releaseAVPacket(&packet);

        if(ret){
            break;
        }

        // 从缓冲区获取原始包
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(codecContext, frame);
        if (ret == AVERROR(EAGAIN)) {
            continue; // 有可能音频帧，也会获取失败，重新拿一次
        } else if (ret != 0) {
            break; // 错误了
        }
        // 音频到原始数据 pcm
        frames.insertToQueue(frame);
    }
    releaseAVPacket(&packet);

}

void AudioChannel::audio_play() {

}
