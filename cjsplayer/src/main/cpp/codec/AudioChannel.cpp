//
// Created by 哦 on 2023/2/15.
//

#include "AudioChannel.h"
// channel


AudioChannel::AudioChannel(int stream_index, AVCodecContext *codecContext, AVRational time_base) : BaseChannel(
        stream_index, codecContext, time_base) {

    // 一些初始化工作
    // AV_CH_LAYOUT_STEREO 双声道类型，前左声道 前右声道
    out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    // 样本大小 16bit == 2字节
    out_sample_size = av_get_bytes_per_sample(AVSampleFormat::AV_SAMPLE_FMT_S16);
    // 采样率
    out_sample_rate = 44100;

    out_buffers_size = out_sample_rate * out_sample_size * out_channels;
    out_buffers = static_cast<uint8_t *>(malloc(out_buffers_size));

    // 音频冲采样上下文
    swr_ctx = swr_alloc_set_opts(nullptr,
                                 // 输出参数
                                 AV_CH_LAYOUT_STEREO, // 声道布局类型，双声道
                                 AV_SAMPLE_FMT_S16, // 采样类型 16bit
                                 out_sample_rate, // 采样率

                                 // 输入参数
                                 codecContext->channel_layout,
                                 codecContext->sample_fmt,
                                 codecContext->sample_rate,
                                 0, nullptr
                                 );

    swr_init(swr_ctx);

}


AudioChannel::~AudioChannel() {

}


void *task_audio_decode(void *args) {
    auto channel = static_cast<AudioChannel *>(args);
    channel->audio_decode();
    return nullptr;
}

void *task_audio_play(void *args) {
    auto channel = static_cast<AudioChannel *>(args);
    channel->audio_play();
    return nullptr;
}


void AudioChannel::start() {
    isPlaying = true;

    //缓冲队列开始工作
    packets.setWork(1);
    frames.setWork(1);

    // 音频解码线程
    pthread_create(&pid_audio_decode, nullptr, task_audio_decode, this);
    // 获取音频原始数据，播放
    pthread_create(&pid_audio_play, nullptr, task_audio_play, this);
}

void AudioChannel::stop() {

}

/**
 *  取出数据压缩包，解码为原始包 AVFrame*
 */
void AudioChannel::audio_decode() {
    AVPacket *packet = nullptr;

    while (isPlaying) {

        if(isPlaying && frames.size() > 100){
            av_usleep(10 * 1000);
            continue;
        }

        int ret = packets.getQueueAndDel(packet);

        if (!isPlaying) {
            break;
        }

        if (!ret) {
            // 获取失败之后继续获取
            continue;
        }



        // 将压缩包发送到缓冲区，再从缓冲区获取到原始包
        ret = avcodec_send_packet(codecContext, packet);

        if (ret) {
            break;
        }

        // 从缓冲区获取原始包
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(codecContext, frame);
        if (ret == AVERROR(EAGAIN)) {
            continue; // 有可能音频帧，也会获取失败，重新拿一次
        } else if (ret != 0) {
            if(frame){
                av_frame_unref(frame);
                releaseAVFrame(&frame);
            }
            break; // 错误了
        }
        // 音频到原始数据 pcm
        frames.insertToQueue(frame);

        av_packet_unref(packet);
        releaseAVPacket(&packet);
    }
    if(packet){
        av_packet_unref(packet);
        releaseAVPacket(&packet);
    }

}





/**
 * 播放回调函数
 * @param bp
 * @param args
 */
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bp,
                       void *args){

    auto channel = static_cast<AudioChannel*>(args);

    int pcm_size = channel->getPCM();

    // 添加数据到缓冲区
    (*bp)->Enqueue(bp,
                   channel->out_buffers,   // pcm 缓冲数据
                   pcm_size // pcm 数据对应的大小
                   );

}


/**
 * 从队列取出音频原始数据pcm，通过 OpenSLES 进行播放
 */
void AudioChannel::audio_play() {
    // 用于接受函数执行到返回值
    SLresult result;

    /**
     * 1 创建引擎对象并获取接口
     */
    // 创建引擎对象
    result = slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("slCreateEngine 创建引擎失败")
        return;
    }
    // 初始化引擎
    // SL_BOOLEAN_FALSE 延时等待创建成功
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("Realize 初始化引擎失败")
        return;
    }

    // 获取引擎接口
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineInterface);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("GetInterface 获取引擎接口失败")
        return;
    }

    if (!engineInterface) {
        LOGE("创建引擎接口失败");
        return;
    }

    LOGI("创建引擎接口成功");

    /**
     * 2 创建混音器
     */

    // 创建混音器
    result = (*engineInterface)->CreateOutputMix(engineInterface, &outputMixObject, 0, nullptr,
                                                 nullptr);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("CreateOutputMix 创建混音器失败")
        return;
    }
    // 初始化混音器 SL_BOOLEAN_FALSE 延时等待创建成功
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);

    if (SL_RESULT_SUCCESS != result) {
        LOGE("outputMixObject Realize 混音器初始化失败");
        return;
    }

    // 混音器接口 处理声音特效
    // 获取混音器接口
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        // 设置混响
        // SL_I3DL2_ENVIRONMENT_PRESET_ROOM 室内效果
        // SL_I3DL2_ENVIRONMENT_PRESET_AUDITORIUM 礼堂等
        const SLEnvironmentalReverbSettings settings = SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT;
        (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &settings);
    }

    LOGI("设置混音器成功");


    /**
     *  3 创建播放器
     */
    // 创建buffer缓冲类型队列
    // 缓冲期类型， 缓冲区数队列大小
    SLDataLocator_AndroidSimpleBufferQueue buffer_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};


    SLDataFormat_PCM format_pcm = {
            SL_DATAFORMAT_PCM, // 数据格式 pcm
            2, // 声道数
            SL_SAMPLINGRATE_44_1, // 采样率（每秒44100个点）
            SL_PCMSAMPLEFORMAT_FIXED_16, // 采样格式，每秒采样样本 存放大小 16bit
            SL_PCMSAMPLEFORMAT_FIXED_16, // 采样样本大小 16bit == 2字节
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, // 前左声道 前右声道
            SL_BYTEORDER_LITTLEENDIAN // 小端模式
    };

    // 数据源
    // 将配置信息放入数据源
    SLDataSource audioSrc = {&buffer_queue, &format_pcm};

    // 配置音频输出的音轨
    // 设置混音器
    // SL_DATALOCATOR_OUTPUTMIX 输出混音器类型
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    // 最终的混音器
    SLDataSink audioSnk = {&loc_outmix, nullptr};

    // 操作队列的接口
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};

    // 创建播放器
    result = (*engineInterface)->CreateAudioPlayer(
            engineInterface, // 引擎接口
            &bqPlayerObject, // 播放器
            &audioSrc,  // 音频配置信息
            &audioSnk,  // 混音器

            // 打开队列的参数
            1, // 开放的参数的个数
            ids, // 代表我们需要buffer
            req // 代表上面的buffer 需要开放出去
            );

    if(SL_RESULT_SUCCESS != result){
        LOGE("创建播放器失败")
        return;
    }

    // 初始化播放器
    // SL_BOOLEAN_FALSE 延时等待创建成功
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    if(SL_RESULT_SUCCESS != result){
        LOGE("播放器初始化失败")
        return;
    }

    LOGD("创建播放器 CreateAudioPlayer success!");

    // 获取播放器接口
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    if(SL_RESULT_SUCCESS != result){
        LOGE("获取播放器接口失败")
        return;
    }

    LOGD("创建播放器 success!");


    /**
     * 4 设置回调函数
     */
     // 获取播放器队列接口
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE, &bqPlayerBufferQueue);
    if(SL_RESULT_SUCCESS != result){
        LOGE("获取播放队列失败");
        return;
    }

    // 设置回调 void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
    (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, this);

    /**
     * 5 设置播放器状态为播放状态
     */
    (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);


    /**
     *  6 手动激活回调函数
     */
     bqPlayerCallback(bqPlayerBufferQueue, this);

}

/**
 * 计算pcm数据大小
 * @return
 */
int AudioChannel::getPCM() {

    int pcm_data_size = 0;

    // 获取 pcm 数据
    AVFrame *frame = nullptr;
    if(isPlaying){
        int ret = frames.getQueueAndDel(frame);

        if(!ret){
            av_frame_unref(frame);
            releaseAVFrame(&frame);
            return 0;
        }

        /**
         * 开始重采样
         */
        // 计算重采样之后的样本数
        // 比如：来源：10个48000   ---->  目标:44100  11个44100
        int dst_nb_samples = av_rescale_rnd(
                // 获取下一个输入样本相对于下一个输出样本将经历的延迟
                swr_get_delay(swr_ctx, frame->sample_rate) + frame->nb_samples,
                // 输出采样率
                out_sample_rate,
                // 输入采样率
                frame->sample_rate,
                AV_ROUND_UP  // 取上，比如10个装不完就取11个
                );

        // 重采样
        // return 返回的结果：每个通道输出的样本数(注意：是转换后的)
        int samples_per_channel = swr_convert(
                swr_ctx,
                // 输出配置
                &out_buffers,// 重采样后的数据buffer
                dst_nb_samples, // 重采样之后的样本数

                // 输入信息参数
                (const uint8_t **)(frame->data), // 没有重采样的pcm数据
                frame->nb_samples // 输入pcm 数据的样本数
                );

        // 由于 out_buffers 和 dst_nb_samples 无法对应，需要重新计算
        // pcm数据大小 = 单通道样本数 * 样本大小(采样位深) * 通道数
        pcm_data_size = samples_per_channel * out_sample_size * out_channels;

        // 音频播放的时间戳
        audio_time = frame->best_effort_timestamp * av_q2d(time_base);

    }

    av_frame_unref(frame);
    releaseAVFrame(&frame);
    return pcm_data_size;
}

