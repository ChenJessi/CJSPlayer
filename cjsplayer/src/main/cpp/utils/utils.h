//
// Created by 哦 on 2023/2/18.
//

#ifndef CJSPLAYER_UTILS_H
#define CJSPLAYER_UTILS_H

#define THREAD_MAIN 1
#define THREAD_CHILD 2


// 打开媒体url 失败
#define CODE_OPEN_URL_FAIL 101
// 无法找到流媒体
#define CODE_FIND_STREAMS_FAIL 102
// 找不到解码器
#define CODE_FIND_DECODER_FAIL 103
// 无法创建解码器上下文
#define CODE_ALLOC_NEW_CODEC_CONTEXT_FAIL 104
// 填充解码器上下文参数失败
#define CODE_FILL_CODEC_CONTEXT_FAIL 105
// 打开解码器失败
#define CODE_OPEN_CODEC_FAIL 106
// 没有音视频媒体信息
#define CODE_NOT_MEDIA 107


#endif //CJSPLAYER_UTILS_H
