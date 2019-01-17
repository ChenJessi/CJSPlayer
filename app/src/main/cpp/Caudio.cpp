//
// Created by CHEN on 2019/1/16.
//

#include "Caudio.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "AndroidLog.h"
//memcpy
#include <string.h>
#include <libavcodec/avcodec.h>

//引擎对象
SLObjectItf enginObject = NULL;
//引擎接口
SLEngineItf engineItf = NULL;

//混音器对象(混音器作用是做声音处理)
SLObjectItf outputmixObject = NULL;
//混音器环境接口
SLEnvironmentalReverbItf outputEnvironmentalReverbItf = NULL;
const SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

//播放器对象
SLObjectItf pcmPlayerObject = NULL;
//播放接口
SLPlayItf playItf = NULL;
//播放队列
SLAndroidSimpleBufferQueueItf simpleBufferQueueItf = NULL;


FILE *pcmFile = NULL;
uint8_t *outbuffer;
void* buffer;

//从pcm文件中读取数据
int getPCMbuffer(void** pcm){
    //读取到的字节数
    int size = 0;
    while (!feof(pcmFile)){
        size = fread(outbuffer,1,44100 * 2 * 2 / 2,pcmFile);
        if (outbuffer==NULL){
            LOGI("pcm文件读取完毕");
            break;
        } else{
            LOGI("pcm文件读取中");
        }
        *pcm = outbuffer;
        break;
    }
    return size;
}

void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf simpleBufferQueueItf1,void* data){
    int size = getPCMbuffer(&buffer);
    LOGI("pcm读取大小为：%d",size);

    if (buffer!=NULL){
        LOGI("放入播放队列");

        //放入播放队列
        (*simpleBufferQueueItf1)->Enqueue(simpleBufferQueueItf1,buffer,size);
    }
}


extern "C"
JNIEXPORT void JNICALL
Java_com_chen_ffmpegPlayer_MainActivity_play(JNIEnv *env, jobject instance, jstring url_) {
    const char *url = env->GetStringUTFChars(url_, 0);

    SLresult result;
    pcmFile = fopen(url,"r");
    if (pcmFile==NULL){
        LOGI("文件不存在");

        return;
    }
    LOGI("文件存在");

    outbuffer = (uint8_t *) malloc(44100 * 2 * 2 / 2);

    //初始化引擎对象并由对象得到接口
    slCreateEngine(&enginObject, 0, 0, 0, 0, 0);
    (*enginObject)->Realize(enginObject, SL_BOOLEAN_FALSE);
    result = (*enginObject)->GetInterface(enginObject, SL_IID_ENGINE, &engineItf);
    if (SL_RESULT_SUCCESS!=result){
        LOGI("引擎接口创建失败！");
        return;
    }
    //需要做的声音处理功能数组
    const SLInterfaceID mid[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mird[1] = {SL_BOOLEAN_FALSE};
    result = (*engineItf)->CreateOutputMix(engineItf, &outputmixObject,1,mid,mird);

    if (SL_RESULT_SUCCESS!=result){
        LOGI("混音器对象创建失败！");
        return;
    }

    //得到上面声明的处理功能的接口
    (*outputmixObject)->Realize(outputmixObject,SL_BOOLEAN_FALSE);
    result = (*outputmixObject)->GetInterface(outputmixObject,SL_IID_ENVIRONMENTALREVERB,&outputEnvironmentalReverbItf);
    if (SL_RESULT_SUCCESS!=result){
        LOGI("混音器接口创建失败！");
        return;
    }
    //混音器环境属性设置
    result = (*outputEnvironmentalReverbItf)->SetEnvironmentalReverbProperties(outputEnvironmentalReverbItf,&reverbSettings);

    if (SL_RESULT_SUCCESS!=result){
        LOGI("混音器参数设置失败！");
    }

    //播放队列
    SLDataLocator_AndroidBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};
    SLDataFormat_PCM format_pcm = {
            SL_DATAFORMAT_PCM, //格式
            2,//声道数
            SL_SAMPLINGRATE_44_1,//采样率
            SL_PCMSAMPLEFORMAT_FIXED_16,//采样位数 一定要与播放的pcm的一样 要不然可能快也可能会慢
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_RIGHT|SL_SPEAKER_FRONT_LEFT,//声道布局 前右|前左
            SL_BYTEORDER_LITTLEENDIAN
    };
    //播放源
    SLDataSource dataSource = {&android_queue,&format_pcm};
    //混音器
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX,outputmixObject};
    //关联混音器
    SLDataSink audiosnk = {&outputMix,NULL};
    //要实现的功能
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};//队列播放
    const SLboolean irds[1] = {SL_BOOLEAN_FALSE};
    (*engineItf)->CreateAudioPlayer(engineItf,&pcmPlayerObject,&dataSource,&audiosnk,1,ids,irds);
    (*pcmPlayerObject)->Realize(pcmPlayerObject,SL_BOOLEAN_FALSE);
    //创建播放接口
    result = (*pcmPlayerObject)->GetInterface(pcmPlayerObject,SL_IID_PLAY,&playItf);

    if (SL_RESULT_SUCCESS!=result){
        LOGI("播放器接口创建失败！");
        return;
    }

    //得到Androidbufferqueue接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject,SL_IID_BUFFERQUEUE,&simpleBufferQueueItf);
    //注册回掉函数
    (*simpleBufferQueueItf)->RegisterCallback(simpleBufferQueueItf,pcmBufferCallBack,NULL);
    //设置播放状态
    (*playItf)->SetPlayState(playItf,SL_PLAYSTATE_PLAYING);

    //第一次手动调用回掉函数 开始播放
    pcmBufferCallBack(simpleBufferQueueItf,NULL);

    env->ReleaseStringUTFChars(url_, url);
}
