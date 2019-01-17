//
// Created by CHEN on 2019/1/14.
//

#ifndef FFMPEGPLAYER_CXVIDEO_H
#define FFMPEGPLAYER_CXVIDEO_H

#include "AndroidLog.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
/**
 * ffmpeg
 */
#include <libavutil/time.h>
//重采样
#include <libswresample/swresample.h>

//引擎对象
SLObjectItf enginObject = NULL;

//引擎接口
SLEngineItf engineItf = NULL;

//混音器对象（混音器作用是做声音处理)
SLObjectItf outputmixObject = NULL;
//混音器环境接口
SLEnvironmentalReverbItf outputEnvironmentalReverbItf = NULL;
const SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

//播放器对象
SLObjectItf pcmPlayerObject = NULL;
//播放接口
SLPlayItf playItf = NULL;
//播放队列   通过此接口往队列写数据，发送的数据播放完后会从缓冲队列中删掉
SLAndroidSimpleBufferQueueItf simpleBufferQueueItf = NULL;

FILE *pcmFile = NULL;
uint8_t *outbuffer;
void *buffer;
class CXVideo {

};


#endif //FFMPEGPLAYER_CXVIDEO_H
