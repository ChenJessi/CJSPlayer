//
// Created by CHEN on 2019/2/26.
//


#include "CyAudio.h"

CyAudio::CyAudio(CyPlaystatus *cyPlaystatus , int sample_rate, CyCallJava *callJava) {
    this->cyPlaystatus = cyPlaystatus;
    this->sample_rate = sample_rate;
    this->callJava = callJava;
    this->isCut = false;
    this->end_time = 0;
    this->showPcm = false;

    queue = new CyQueue(cyPlaystatus);
    bufferQueue = new CyBufferQueue(cyPlaystatus);
    buffer = (uint8_t *)(av_malloc(sample_rate * 2 * 2));

    sampleBuffer = static_cast<SAMPLETYPE *>(malloc(sample_rate * 2 * 2));
    soundTouch = new SoundTouch();
    soundTouch->setSampleRate(sample_rate);
    soundTouch->setChannels(2);
    soundTouch->setPitch(pitch);
    soundTouch->setTempo(speed);

    pthread_mutex_init(&sound_mutex, NULL);
    pthread_mutex_init(&sles_mutex, NULL);
}

CyAudio::~CyAudio() {
    pthread_mutex_destroy(&sound_mutex);
    pthread_mutex_destroy(&sles_mutex);
}

void *decodPlay(void *data){
    CyAudio *cyAudio = (CyAudio *)data;

    cyAudio->initOpenSLES();

    pthread_exit(&cyAudio->thread_play);
}
void *pcmCallBack(void *data){
    CyAudio *audio = static_cast<CyAudio *>(data);
    while (audio->cyPlaystatus != NULL && !audio->cyPlaystatus->exit){
        CyPcmBean *pcmBean = NULL;
        audio->bufferQueue->getBuffer(&pcmBean);
        if (pcmBean == NULL){
            continue;
        }
       if (pcmBean->buffersize <= audio->defaultPcmSize){
           if (audio->isRecordPcm){
               audio->callJava->onCallPcmToAAC(CHILD_THREAD, pcmBean->buffersize, audio->buffer);
           }
           if (audio->showPcm){
               audio->callJava->onCallPcmInfo(pcmBean->buffersize, audio->buffer);
           }
       } else{
           int pack_num = pcmBean->buffersize / audio->defaultPcmSize;
           int pack_sub = pcmBean->buffersize % audio->defaultPcmSize;
           for (int i = 0; i < pack_num; i++) {
               char *bf = static_cast<char *>(malloc(audio->defaultPcmSize));
               memcpy(bf, pcmBean->buffer + i * audio->defaultPcmSize, audio->defaultPcmSize);
               if (audio->isRecordPcm){
                   audio->callJava->onCallPcmToAAC(CHILD_THREAD, audio->defaultPcmSize, bf);
               }
               if (audio->showPcm){
                   audio->callJava->onCallPcmInfo(pcmBean->buffersize, audio->buffer);
               }
               free(bf);
               bf = NULL;
           }
           if (pack_sub > 0){
               char *bf = static_cast<char *>(malloc(pack_sub));
               memcpy(bf, pcmBean->buffer + pack_num * audio->defaultPcmSize, pack_sub);
               if (audio->isRecordPcm){
                   audio->callJava->onCallPcmToAAC(CHILD_THREAD, pack_sub, bf);
               }
               if (audio->showPcm){
                   audio->callJava->onCallPcmInfo(pack_sub, bf);
               }
           }
       }
       delete(pcmBean);
       pcmBean = NULL;
    }
    pthread_exit(&audio->pcmCallBackThread);
};
void CyAudio::play() {
    pthread_create(&thread_play, NULL, decodPlay, this);
    pthread_create(&pcmCallBackThread, NULL, pcmCallBack , this);
}

int CyAudio::resampleAudio(void **pcmbuf) {
    data_size = 0;
    while (cyPlaystatus != NULL && !cyPlaystatus->exit){
        if (cyPlaystatus->seek){
            av_usleep(1000 * 100);
            continue;
        }
        if (queue->getQueueSize() == 0){ //加载中
            if (!cyPlaystatus->load){
                cyPlaystatus->load = true;
                callJava->onCallLoad(CHILD_THREAD, true);
            }
            av_usleep(1000 * 100);
            continue;
        } else{
            if (cyPlaystatus->load){
                cyPlaystatus->load = false;
                callJava->onCallLoad(CHILD_THREAD, false);
            }
        }
        if (readFrameFinished){
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
        }

        avFrame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, avFrame);
        if (ret == 0){
            readFrameFinished = false;
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
                readFrameFinished = true;
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

            if (now_time < clock){
                now_time = clock;
            }
            clock = now_time;

            *pcmbuf = buffer;

            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            swr_free(&swr_ctx);
            break;
        } else{
            readFrameFinished = true;
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
    CyAudio *cyAudio = (CyAudio *)(context);
    // for streaming playback, replace this test by logic to find and fill the next buffer
    if (cyAudio != NULL){
        int samplesize = cyAudio->getSoundTouchData();

        if (samplesize > 0){
            cyAudio->clock += samplesize / ((double)(cyAudio->sample_rate * 2 * 2));
           if (cyAudio->clock - cyAudio->last_time >= 0.1){

               cyAudio->last_time = cyAudio->clock;

               cyAudio->callJava->onCallTimeInfo(CHILD_THREAD, cyAudio->clock, cyAudio->duration);
           }
            cyAudio->bufferQueue->putBuffer(cyAudio->sampleBuffer,samplesize * 4);
           cyAudio->callJava->onCallValumeDB(CHILD_THREAD,
           cyAudio->getPCMDB(reinterpret_cast<char *>(cyAudio->sampleBuffer), samplesize * 4));
            (* cyAudio->pcmBufferQueue)->Enqueue(cyAudio->pcmBufferQueue, (char *)cyAudio->sampleBuffer, samplesize * 2 * 2);
            if (cyAudio->isCut){
                if (cyAudio->clock > cyAudio->end_time){
                    LOGE("裁剪退出...")
                    cyAudio->cyPlaystatus->exit = true;
                }
            }
        }
    }

};

void CyAudio::initOpenSLES() {

    SLresult  result;
    //d第一步---------------------------------------------------------
    //创建引擎对象
    result = slCreateEngine(&engineObject,0,0,0,0,0);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("slCreateEngine failed.");
        return ;
    }
    (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineItf);

    //第二步-----------------------------------------------------------------
    //创建混音器
    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};
    result = (*engineItf)->CreateOutputMix(engineItf, &outputMixObject, 1, mids, mreq);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("CreateOutputMix failed.");
        return ;
    }
    (void)result;
    result = (*outputMixObject)->Realize(outputMixObject,SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("Realize OutputMix failed.");
        return ;
    }
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

    const SLInterfaceID  ids[4] = {SL_IID_BUFFERQUEUE , SL_IID_MUTESOLO , SL_IID_PLAYBACKRATE ,SL_IID_VOLUME};
    const SLboolean req[4] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    result = (*engineItf)->CreateAudioPlayer(engineItf,  &pcmPlayerObject, &slDataSource, &audioSnk, 4, ids, req);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("CreateAudioPlayer failed.");
        return ;
    }
    //初始化播放器
    result = (*pcmPlayerObject)->Realize(pcmPlayerObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("Realize Player failed.");
        return ;
    }
    //得到接口后调用 获取player接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_PLAY, &pcmPlayerPlay);
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_VOLUME, &pcmVolumePlay);
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_MUTESOLO, &pcmMutePlay);
    //第四步----------------------------------------------------------------------------------------
    //创建缓冲区和回调函数
    result = (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_BUFFERQUEUE ,&pcmBufferQueue);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("create BufferQueue failed.");
//        return ;
    }
    setVolume(volumePercent);
    setMute(mute);
    //缓冲接口回调
    (*pcmBufferQueue)->RegisterCallback(pcmBufferQueue, pcmBufferCallBack,this);
    //设置播放状态
    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay,  SL_PLAYSTATE_PLAYING);
    LOGD("初始化完成");
    pcmBufferCallBack(pcmBufferQueue, this);

}

SLuint32 CyAudio::getCurrentSampleRateForOpensles(int sample_rate) {
    int rate = 0;
    switch (sample_rate) {
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

    if(bufferQueue != NULL) {
        bufferQueue->noticeThread();
        pthread_join(pcmCallBackThread, NULL);
        bufferQueue->release();
        delete(bufferQueue);
        bufferQueue = NULL;
    }
    if (queue != NULL){
        delete(queue);
        queue = NULL;
    }
    if (pcmPlayerObject != NULL){
        (*pcmPlayerObject)->Destroy(pcmPlayerObject);
        pcmPlayerObject = NULL;
        pcmPlayerPlay = NULL;
        pcmBufferQueue = NULL;
        pcmMutePlay = NULL;
        pcmVolumePlay = NULL;
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
    if(out_buffer != NULL) {
        out_buffer = NULL;
    }
    if(soundTouch == NULL) {
        delete(soundTouch) ;
        soundTouch = NULL;
    }
    if(sampleBuffer != NULL) {
        free(sampleBuffer);
        sampleBuffer = NULL;
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
    pthread_mutex_unlock(&sound_mutex);
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
    for(int i = 0; i < pcmsize; i+= 2) {
        memcpy(&pervalue, pcmdata+i, 2);
        sum += abs(pervalue);
    }
    sum = sum / (pcmsize / 2);
    if(sum > 0) {
        db = (int)20.0 *log10(sum);
    }
    return db;
}

void CyAudio::startStopRecord(bool start) {
    this->isRecordPcm = start;
}
