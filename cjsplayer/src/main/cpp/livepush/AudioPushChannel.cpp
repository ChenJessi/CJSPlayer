//
// Created by 哦 on 2023/4/9.
//

#include "AudioPushChannel.h"

AudioPushChannel::AudioPushChannel() {

}

AudioPushChannel::~AudioPushChannel() {

}

void AudioPushChannel::setAudioCallback(AudioPushChannel::AudioCallback callback) {
    audioCallback = callback;
}

void AudioPushChannel::initAudioEncoder(int sample_rate, int num_channel) {

}
