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

void AudioChannel::start() {

}

void AudioChannel::stop() {

}
