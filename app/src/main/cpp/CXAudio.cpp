//
// Created by CHEN on 2019/1/14.
//
#include "CXAudio.h"




//从pcm文件中读取数据
int getPCMbuffer(void **pcm) {
    //d读取到的字节数
    int size = 0;
    while (!feof(pcmFile)) {
        size = fread(outbuffer, 1, 44100 * 2 * 2 / 2, pcmFile);
        if (outbuffer == NULL) {
            LOGI("pcm文件读取完毕");
            break;
        } else {
            LOGI("PCM文件读取中")
        }
        *pcm = outbuffer;
        break;
    }
    return size;
}

/**
 * 播放回调
 */
void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf simpleBufferQueueItf, void *data) {
    int size = getPCMbuffer(&buffer);
    LOGI("pcm读取大小为：%d", size);

    if (buffer != NULL) {
        LOGI("放入播放队列");
        /*
        SLresult (*Enqueue) ( // 给队列传递数据
            SLAndroidSimpleBufferQueueItf simpleBufferQueueItf, // 队列
            const void *buffer, // 要传送的数据
            SLuint32 size // 数据的长度
        );
     */
        (*simpleBufferQueueItf)->Enqueue(simpleBufferQueueItf, buffer, size);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_chen_ffmpegPlayer_Audio_AudioPlayer_playPCM(JNIEnv *env, jclass type, jstring url_) {
    const char *url = env->GetStringUTFChars(url_, 0);
//    // TODO
    SLresult result;
    pcmFile = fopen(url, "r");       //只读方式打开
    if (pcmFile == NULL) {
        LOGI("文件不存在");
        return;
    }
    LOGI("文件存在");

    outbuffer = (uint8_t *) malloc(44100 * 2 * 2 / 2);

    /*
       SL_API SLresult SLAPIENTRY slCreateEngine( // 创建OpenSLES引擎
           SLObjectItf             *pEngine, // 上下文
           SLuint32                numOptions, // 选择项的数量
           const SLEngineOption    *pEngineOptions, // 具体的选择项
           SLuint32                numInterfaces, // 支持的接口的数量
           const SLInterfaceID     *pInterfaceIds, // 具体的要支持的接口，是枚举的数组
           const SLboolean         * pInterfaceRequired // 具体的要支持的接口是开放的还是关闭的，也是一个数组，这三个参数长度是一致的
       );
    */




    //初始化引擎对象并由对象得到接口
    result = slCreateEngine(&enginObject, 0, 0, 0, 0, 0);
    //实现（Realize）engineObject接口对象
    result = (*enginObject)->Realize(enginObject, SL_BOOLEAN_FALSE);
    //通过engineObject的GetInterface方法初始化engineEngine
    result = (*enginObject)->GetInterface(enginObject, SL_IID_ENGINE, &engineItf);
    if (SL_RESULT_SUCCESS != result) {
        LOGI("引擎接口创建失败")
        return;
    }
    LOGI("引擎接口创建成功")


    //创建混音器
    const SLInterfaceID mid[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mird[1] = {SL_BOOLEAN_FALSE};

    /*
        SLresult (*CreateOutputMix) ( // 创建混音器
            SLEngineItf engineItf, // 引擎
            SLObjectItf * outputmixObject, // 混音器，输入输出参数
            SLuint32 numInterfaces, // 支持的接口的数量
            const SLInterfaceID * mid, // 具体的要支持的接口，是枚举的数组
            const SLboolean * mird // 具体的要支持的接口是开放的还是关闭的，也是一个数组，这三个参数长度是一致的
        );
     */
    result = (*engineItf)->CreateOutputMix(engineItf, &outputmixObject, 1, mid, mird);

    if (SL_RESULT_SUCCESS != result) {
        LOGI("混音器对象创建失败！");
        return;
    }
    LOGI("混音器对象创建成功！");



    //得到上面声明的处理功能的接口
    /*
        SLresult (*Realize) ( // 实例化混音器
            SLObjectItf self, // 混音器本身
            SLboolean async // SL_BOOLEAN_FALSE 表示阻塞知道初始化完成，设置为 SL_BOOLEAN_TURE 时立即返回成功，这个时候初始化的执行是异步的
        );
     */
   (*outputmixObject)->Realize(outputmixObject, SL_BOOLEAN_FALSE);
    result = (*outputmixObject)->GetInterface(outputmixObject,SL_IID_ENVIRONMENTALREVERB,&outputEnvironmentalReverbItf);
    if (SL_RESULT_SUCCESS != result) {
        LOGI("混音器接口创建失败!");
        return;
    }
    LOGI("混音器接口创建成功！");
    //混音器环境属性设置

    result = (*outputEnvironmentalReverbItf)->SetEnvironmentalReverbProperties(
            outputEnvironmentalReverbItf, &reverbSettings);
    if (SL_RESULT_SUCCESS != result) {
        LOGI("混音器参数设置失败！");
    }
    LOGI("混音器参数设置成功！");




    // 缓冲队列的配置信息，用队列的方式在cpu与音频播放设备之间来交互，SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE 是固定传入，10表示队列的长度
    SLDataLocator_AndroidBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};

    SLDataFormat_PCM format_pcm = {
            SL_DATAFORMAT_PCM,//输入的音频格式，这里只能是SL_DATAFORMAT_PCM，表示PCM格式的原始未压缩的数据，其他格式的用其他格式对应的结构体?
            2, // 2个声道数(立体声)
            SL_SAMPLINGRATE_44_1,//采样率44100hz
            SL_PCMSAMPLEFORMAT_FIXED_16,// 采样位数16bit保存一个音   一定要与播放的pcm的一样 要不然可能快也可能会慢
            SL_PCMSAMPLEFORMAT_FIXED_16,// 容器大小，与 bitsPerSample 一样大可以了
            SL_SPEAKER_FRONT_RIGHT | SL_SPEAKER_FRONT_LEFT,//声道布局   前左声道与前右声道，  左右声道(立体声)
            SL_BYTEORDER_LITTLEENDIAN  //字节序， SL_BYTEORDER_LITTLEENDIAN 表示小端 ，播放的声音有问题的时候，这个是一个排查的方向
    };

    //播放源
    SLDataSource dataSource = {&android_queue, &format_pcm};
    //混音器     // SLDataLocator_OutputMix 表示输出的对象，SL_DATALOCATOR_OUTPUTMIX是固定的传这个
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputmixObject};
    //关联混音器   // audioSink 表示声音的对象，播放的时候传进去，表示播放的是哪个混音设备
    SLDataSink audiosnk = {&outputMix, NULL};




    //要实现的功能
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};// SL_IID_BUFFERQUEUE 表示缓冲队列的接口
    const SLboolean irds[1] = {SL_BOOLEAN_FALSE};  // 表示ids中需要获取的接口是开放还是关闭，这两个数组要长度一致
    /*
        SLresult (*CreateAudioPlayer) ( // 创建播放器
        SLEngineItf engineItf, // 播放引擎
        SLObjectItf * pPlayer, // 创建的播放器，输入输出参数
        SLDataSource *pAudioSrc, // 数据源的配置信息
        SLDataSink *audiosnk, // 输出的设备的配置信息
        SLuint32 numInterfaces, // 支持的接口的数量
        const SLInterfaceID * ids, // 具体的要支持的接口，是枚举的数组
        const SLboolean * irds // 具体的要支持的接口是开放的还是关闭的，也是一个数组，这三个参数长度是一致的
        );
    */
    (*engineItf) ->CreateAudioPlayer(engineItf,&pcmPlayerObject,&dataSource,&audiosnk,1,ids,irds);

    (*pcmPlayerObject) ->Realize(pcmPlayerObject,SL_BOOLEAN_FALSE); // 阻塞等待实例化完成

    /*
        SLresult (*GetInterface) ( // 创建播放接口
            SLObjectItf pcmPlayerObject, // 上下文
            const SLInterfaceID SL_IID_PLAY, // 要获取的接口类型
            void * pInterface // 获取到的接口存储到这里面
        );
     */
    // SL_IID_PLAY 表示要获取播放接口，CreateAudioPlayer函数中不需要设置支持它
    result = (*pcmPlayerObject) ->GetInterface(pcmPlayerObject, SL_IID_PLAY, &playItf);
    if (SL_RESULT_SUCCESS != result){
        LOGI("播放器接口创建失败！");
        return;
    }
    LOGI("播放器接口创建成功！");

    //获取播放的缓冲队列接口，用于注册缓冲队列接口的回调函数
    result = (*pcmPlayerObject) ->GetInterface(pcmPlayerObject,SL_IID_BUFFERQUEUE,&simpleBufferQueueItf);
    if (SL_RESULT_SUCCESS != result){
        LOGI("获取播放缓冲队列失败！");
        return;
    }
    LOGI("获取播放缓冲队列成功！");

    /*
        SLresult (*RegisterCallback) ( // 设置缓冲接口的回调函数，在播放队列为空的时候调用
            SLAndroidSimpleBufferQueueItf simpleBufferQueueItf,
            SlAndroidSimpleBufferQueueCallback callback, // 回调函数
            void* pContext // 给回调函数传的参数，看情况设置
        );
     */
    (*simpleBufferQueueItf) ->RegisterCallback(simpleBufferQueueItf,pcmBufferCallBack,NULL);// env->ReleaseStringUTFChars(path_, path); 可能会在回调函数前调用，需要注意

    /*
       SLresult (*SetPlayState) ( // 设置播放状态
           SLPlayItf playItf, // 播放接口
           SLuint32 state // 播放的状态，有 SL_PLAYSTATE_PLAYING SL_PLAYSTATE_PAUSED SL_PLAYSTATE_STOPPED 三种状态
       );
    */
    (*playItf) ->SetPlayState(playItf,SL_PLAYSTATE_PLAYING);// 设置播放状态，播放中，这个时候播放队列的回调函数并不会被调用

    //第一次手动调用回掉函数 开始播放
    pcmBufferCallBack(simpleBufferQueueItf,NULL);

    env->ReleaseStringUTFChars(url_, url);






}

