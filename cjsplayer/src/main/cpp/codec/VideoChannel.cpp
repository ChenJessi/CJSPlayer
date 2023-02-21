//
// Created by 哦 on 2023/2/15.
//

#include "VideoChannel.h"

VideoChannel::VideoChannel(int stream_index, AVCodecContext *codecContext) : BaseChannel(
        stream_index, codecContext) {

}

VideoChannel::~VideoChannel() {

}

void VideoChannel::start() {

}

void VideoChannel::stop() {

}
