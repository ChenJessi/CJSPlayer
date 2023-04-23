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
    this->mChannels = num_channel;

    /**
     * 1. 初始化编码器
     */
    audioEncoder = faacEncOpen(sample_rate, num_channel, &inputSamples, &maxOutputBytes);
    if (!audioEncoder) {
        LOGE("faacEncOpen failed");
        return;
    }

    /**
     * 2. 设置编码器参数
     */
    // 2.1 获取编码器的配置参数
    faacEncConfigurationPtr config = faacEncGetCurrentConfiguration(audioEncoder);
    // 2.2 设置编码器的配置参数
    config->mpegVersion = MPEG4; // MPEG-4
    config->aacObjectType = LOW; // AAC-LC
    config->inputFormat = FAAC_INPUT_16BIT; // 输入的采样精度
    config->outputFormat = 0; // 输出的格式 0为RAW，1为ADTS

    //回音消除 降噪
    config->useTns = 1; // 使用时域噪音控制
    config->useLfe = 0; // 不使用LFE

    // 设置参数
    int ret = faacEncSetConfiguration(audioEncoder, config);
    if (!ret) {
        LOGE("faacEncSetConfiguration failed");
        return;
    }
    LOGE("faacEncSetConfiguration success")
    /**
     * 3. 初始化编码器输出缓冲区
     */
    buffer = new u_char[maxOutputBytes];
}

int AudioPushChannel::getInputSamples() {
    //一般情况是2048单通道
    return inputSamples;
}
