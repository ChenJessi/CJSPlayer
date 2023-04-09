//
// Created by 哦 on 2023/4/9.
//

#include "VideoPushChannel.h"

VideoPushChannel::VideoPushChannel() {

}

VideoPushChannel::~VideoPushChannel() {

}

void VideoPushChannel::initVideoEncoder(int width, int height, int fps, int bitrate) {

}

void VideoPushChannel::setVideoCallback(VideoPushChannel::VideoCallback callback) {
    videoCallback = callback;
}

void VideoPushChannel::encodeData(signed char *data) {

}
