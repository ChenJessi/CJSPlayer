//
// Created by 哦 on 2023/4/9.
//

#include "AudioPushChannel.h"

AudioPushChannel::AudioPushChannel() {

}

AudioPushChannel::~AudioPushChannel() {
    if (buffer){
        delete buffer;
        buffer = nullptr;
    }
    if (audioEncoder){
        faacEncClose(audioEncoder);
        audioEncoder = nullptr;
    }
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

//  faac 音频编码 int8_t *data 无符号
void AudioPushChannel::encodeData(int8_t *data) {

    /**
     * 4. 编码
     *  byteLen 编码后的字节数
     */
    int byteLen = faacEncEncode(audioEncoder, reinterpret_cast<int32_t *>(data), inputSamples, buffer, maxOutputBytes);

    if (byteLen > 0){
        RTMPPacket *packet = new RTMPPacket;
        int bodySize = 2 + byteLen; // 2是ADTS头的大小
        RTMPPacket_Alloc(packet, bodySize);

        packet->m_body[0] = 0xAF; // 1010 1111 // 双声道
        if (mChannels == 1){
            packet->m_body[1] = 0xAE; // 1010 1110
        }

        // 0x01： 音频数据 非序列/非头参数
        packet->m_body[1] |= 0x1;

        // 2. 复制编码后的数据
        memcpy(&packet->m_body[2], buffer, byteLen);

        packet->m_packetType = RTMP_PACKET_TYPE_AUDIO; // 音频类型
        packet->m_nBodySize = bodySize; // 包大小
        packet->m_nChannel = 0x15; // 通道ID
        packet->m_nTimeStamp = -1; // 数据帧有时间戳
        packet->m_hasAbsTimestamp = 0; // 没有绝对时间戳
        packet->m_headerType = RTMP_PACKET_SIZE_LARGE; // 大包

        audioCallback(packet);
    }

}
