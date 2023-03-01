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

void AudioChannel::audio_decode() {



}

void AudioChannel::audio_play() {

}
