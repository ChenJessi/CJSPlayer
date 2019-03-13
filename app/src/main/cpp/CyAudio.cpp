//
// Created by CHEN on 2019/2/26.
//

#include "CyAudio.h"

CyAudio::CyAudio(CyPlaystatus *cyPlaystatus , int sample_rate, CyCallJava *callJava) {
    this->cyPlaystatus = cyPlaystatus;
    queue = new CyQueue(cyPlaystatus);
    this->sample_rate = sample_rate;
    this->callJava = callJava;
    buffer = (uint8_t *)(av_malloc(sample_rate * 2 * 2));

    sampleBuffer = static_cast<SAMPLETYPE *>(malloc(sample_rate * 2 * 2));
    soundTouch = new SoundTouch();
    soundTouch->setSampleRate(sample_rate);
    soundTouch->setChannels(2);
    soundTouch->setPitch(pitch);
    soundTouch->setTempo(speed);

    pthread_mutex_init(&sound_mutex, NULL);
}

CyAudio::~CyAudio() {
    pthread_mutex_destroy(&sound_mutex);
}

void *decodPlay(void *data){
    CyAudio *cyAudio = (CyAudio *)data;

    cyAudio->initOpenSLES();

    pthread_exit(&cyAudio->thread_play);
}

void CyAudio::play() {
    pthread_create(&thread_play, NULL, decodPlay, this);

}

int CyAudio::resampleAudio(void **pcmbuf) {
    data_size = 0;
    while (cyPlaystatus != NULL && !cyPlaystatus->exit){
        if (cyPlaystatus->seek){
            continue;
        }
        if (queue->getQueueSize() == 0){ //加载中
            if (!cyPlaystatus->load){
                cyPlaystatus->load = true;
                callJava->onCallLoad(CHILD_THREAD, true);
            }
            continue;
        } else{
            if (cyPlaystatus->load){
                cyPlaystatus->load = false;
                callJava->onCallLoad(CHILD_THREAD, false);
            }
        }
        avPacket = av_packet_alloc();
        if (queue->getAvpacket(avPacket) != 0){
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        ret = avcodec_send_packet(avCodecContext, avPacket);
        if (ret != 0){
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        avFrame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, avFrame);
        if (ret == 0){
            if (avFrame->channels && avFrame->channel_layout == 0){
                avFrame->channel_layout = av_get_default_channel_layout(avFrame->channels);
            } else if (avFrame->channels == 0 && avFrame->channel_layout > 0){
                avFrame->channels = av_get_channel_layout_nb_channels(avFrame->channel_layout);
            }

            SwrContext *swr_ctx;
            swr_ctx = swr_alloc_set_opts(
                    NULL,
                    AV_CH_LAYOUT_STEREO,
                    AV_SAMPLE_FMT_S16,
                    avFrame->sample_rate,
                    avFrame->channel_layout,
                    (AVSampleFormat) avFrame->format,
                    avFrame->sample_rate,
                    NULL, NULL);
            if (!swr_ctx || swr_init(swr_ctx) < 0){
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;
                swr_free(&swr_ctx);
                continue;
            }
            nb = swr_convert(
                    swr_ctx,
                    &buffer,
                    avFrame->nb_samples,
                    (const uint8_t **)avFrame->data,
                    avFrame->nb_samples);

            int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            data_size = nb * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

            now_time = avFrame->pts * av_q2d(time_base);
            LOGD("now_time ：%d" ,(int)now_time)
            LOGD("now_time clock ：%d" ,(int)clock)
            if (now_time < clock){
                now_time = clock;
            }
            clock = now_time;

            *pcmbuf = buffer;
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            swr_free(&swr_ctx);
            break;
        } else{
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            continue;
        }
    }
    return data_size;
}


void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bf, void * context){
    //assert(null == context)
    CyAudio *cyAudio = (CyAudio *)(context);
    // for streaming playback, replace this test by logic to find and fill the next buffer
    if (cyAudio != NULL){
        int buffersize = cyAudio->getSoundTouchData();
        if (buffersize > 0){
            LOGD("clock ：%d" ,(int)cyAudio->clock)
            cyAudio->clock += buffersize / ((double)(cyAudio->sample_rate * 2 * 2));

           if (cyAudio->clock - cyAudio->last_time >= 0.1){

               cyAudio->last_time = cyAudio->clock;

               cyAudio->callJava->onCallTimeInfo(CHILD_THREAD, cyAudio->clock, cyAudio->duration);
           }
           cyAudio->callJava->onCallValumeDB(CHILD_THREAD,
             cyAudio->getPCMDB(reinterpret_cast<char *>(cyAudio->sampleBuffer), buffersize * 4));
            (* cyAudio->pcmBufferQueue)->Enqueue(cyAudio->pcmBufferQueue, (char *)cyAudio->sampleBuffer, buffersize * 2 * 2);
        }
        pthread_mutex_unlock(&cyAudio->sound_mutex);
    }

};

void CyAudio::initOpenSLES() {

    SLresult  result;
    //d第一步---------------------------------------------------------
    //创建引擎对象
    slCreateEngine(&engineObject,0,0,0,0,0);
    (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineItf);

    //第二步-----------------------------------------------------------------
    //创建混音器
    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};
    result = (*engineItf)->CreateOutputMix(engineItf, &outputMixObject, 1, mids, mreq);
    (void)result;
    result = (*outputMixObject)->Realize(outputMixObject,SL_BOOLEAN_FALSE);
    (void)result;
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB, &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result){
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(outputMixEnvironmentalReverb, &reverbSettings);
        (void)result;
    }
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&outputMix, 0};

    //第三步--------------------------------------------------------------------------
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,  // 播放pcm格式数据
            2,  //2个声道（立体声）
            getCurrentSampleRateForOpensles(sample_rate), //44100hz的频率
            SL_PCMSAMPLEFORMAT_FIXED_16, //位数 16位
            SL_PCMSAMPLEFORMAT_FIXED_16, //和位数一致就行
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, //立体声（前左前右）
            SL_BYTEORDER_LITTLEENDIAN  //结束标志
    };
    SLDataSource slDataSource = {&android_queue, &pcm};

    const SLInterfaceID  ids[3] = {SL_IID_BUFFERQUEUE , SL_IID_MUTESOLO , SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    (*engineItf)->CreateAudioPlayer(engineItf,  &pcmPlayerObject, &slDataSource, &audioSnk, 3, ids, req);
    //初始化播放器
    (*pcmPlayerObject)->Realize(pcmPlayerObject, SL_BOOLEAN_FALSE);

    //得到接口后调用 获取player接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_PLAY, &pcmPlayerPlay);
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_VOLUME, &pcmVolumePlay);
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_MUTESOLO, &pcmMutePlay);
    //第四步----------------------------------------------------------------------------------------
    //创建缓冲区和回调函数
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_BUFFERQUEUE ,&pcmBufferQueue);
    setVolume(volumePercent);
    setMute(mute);
    //缓冲接口回调
    (*pcmBufferQueue)->RegisterCallback(pcmBufferQueue, pcmBufferCallBack,this);

    //设置播放状态
    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay,  SL_PLAYSTATE_PLAYING);

    pcmBufferCallBack(pcmBufferQueue, this);
}

SLuint32 CyAudio::getCurrentSampleRateForOpensles(int sample_rate) {
    int rate = 0;
    switch (sample_rate)
    {
        case 8000:
            rate = SL_SAMPLINGRATE_8;
            break;
        case 11025:
            rate = SL_SAMPLINGRATE_11_025;
            break;
        case 12000:
            rate = SL_SAMPLINGRATE_12;
            break;
        case 16000:
            rate = SL_SAMPLINGRATE_16;
            break;
        case 22050:
            rate = SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            rate = SL_SAMPLINGRATE_24;
            break;
        case 32000:
            rate = SL_SAMPLINGRATE_32;
            break;
        case 44100:
            rate = SL_SAMPLINGRATE_44_1;
            break;
        case 48000:
            rate = SL_SAMPLINGRATE_48;
            break;
        case 64000:
            rate = SL_SAMPLINGRATE_64;
            break;
        case 88200:
            rate = SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            rate = SL_SAMPLINGRATE_96;
            break;
        case 192000:
            rate = SL_SAMPLINGRATE_192;
            break;
        default:
            rate =  SL_SAMPLINGRATE_44_1;
    }
    return rate;
}

void CyAudio::pause() {

    if (pcmPlayerPlay != NULL){
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay,SL_PLAYSTATE_PAUSED);
    }
}

void CyAudio::resume() {
    if (pcmPlayerPlay != NULL){
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay,SL_PLAYSTATE_PLAYING);
    }
}

void CyAudio::stop() {
    if (pcmPlayerPlay != NULL){
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay,SL_PLAYSTATE_STOPPED);
    }
}

void CyAudio::release() {

    if (queue != NULL){
        delete(queue);
        queue = NULL;
    }
    if (pcmPlayerObject != NULL){
        (*pcmPlayerObject)->Destroy(pcmPlayerObject);
        pcmPlayerObject = NULL;
        pcmPlayerPlay = NULL;
        pcmBufferQueue = NULL;
    }
    if (outputMixObject != NULL){
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentalReverb = NULL;
    }
    if (engineObject != NULL){
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineItf = NULL;
    }
    if (buffer != NULL){
        free(buffer);
        buffer = NULL;
    }
    if (avCodecContext != NULL){
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = NULL;
    }
    if (cyPlaystatus != NULL){
        cyPlaystatus = NULL;
    }
    if (callJava != NULL){
        callJava = NULL;
    }
}

void CyAudio::setVolume(int percent) {
    volumePercent = percent;
    if(pcmVolumePlay != NULL) {
        if(percent > 30) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -20);
        } else if(percent > 25) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -22);
        } else if(percent > 20) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -25);
        } else if(percent > 15) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -28);
        } else if(percent > 10) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -30);
        } else if(percent > 5) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -34);
        } else if(percent > 3) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -37);
        } else if(percent > 0) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -40);
        } else{
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -100);
        }
    }
}

void CyAudio::setMute(int mute) {
    this->mute = mute;
    if(pcmMutePlay != NULL){
        if (mute == 0){  //right
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 1, false);
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 0, true);
        }else if(mute == 1){ //left
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 1, true);
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 0, false);
        } else if(mute == 2){ //center
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 1, false);
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 0, false);
        }
    }
}

int CyAudio::getSoundTouchData() {
    pthread_mutex_lock(&sound_mutex);
    while (cyPlaystatus != NULL && !cyPlaystatus->exit){
        out_buffer = NULL;
        if (finished){
            finished = false;
            data_size = resampleAudio(reinterpret_cast<void **>(&out_buffer));

            if (data_size > 0){
                for (int i = 0; i < data_size / 2 + 1; i++) {
                    sampleBuffer[i] = (out_buffer[i * 2] | ((out_buffer[i * 2 + 1]) << 8));
                }
                soundTouch->putSamples(sampleBuffer, nb);
                num = soundTouch->receiveSamples(sampleBuffer, data_size/4);

            } else{
                soundTouch->flush();
            }
        }
        if (num == 0){
            finished = true;
            continue;
        } else{
            if (out_buffer == NULL){
                num = soundTouch->receiveSamples(sampleBuffer, data_size / 4);
                if (num == 0){
                    finished = true;
                    continue;
                }
            }
            pthread_mutex_unlock(&sound_mutex);
            return num;
        }
    }
    return 0;

}

void CyAudio::setPitch(float pitch) {
    this->pitch = pitch;
    if (soundTouch != NULL){
        pthread_mutex_lock(&sound_mutex);
        soundTouch->setPitch(pitch);
        pthread_mutex_unlock(&sound_mutex);
    }
}

void CyAudio::setSpeed(float speed) {
    this->speed = speed;
    if (soundTouch != NULL){
        pthread_mutex_lock(&sound_mutex);
        soundTouch->setTempo(speed);
        pthread_mutex_unlock(&sound_mutex);
    }
}

int CyAudio::getPCMDB(char *pcmdata, size_t pcmsize) {
    int db = 0;
    short int pervalue = 0;
    double sum = 0;
    for(int i = 0; i < pcmsize; i+= 2)
    {
        memcpy(&pervalue, pcmdata+i, 2);
        sum += abs(pervalue);
    }
    sum = sum / (pcmsize / 2);
    if(sum > 0)
    {
        db = (int)20.0 *log10(sum);
    }
    return db;
}
