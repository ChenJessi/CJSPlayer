//
// Created by CHEN on 2019/3/17.
//

#include "CyBufferQueue.h"
#include "AndroidLog.h"

CyBufferQueue::CyBufferQueue(CyPlaystatus *playstatus) {
 this->cyPlaystatus = playstatus;
 pthread_mutex_init(&mutexBuffer, NULL);
 pthread_cond_init(&condBuffer, NULL);
}

CyBufferQueue::~CyBufferQueue() {
    cyPlaystatus = NULL;
    pthread_mutex_destroy(&mutexBuffer);
    pthread_cond_destroy(&condBuffer);
    if (LOG_DEBUG){
        LOGD("cyBufferQueue release")
    }
}

int CyBufferQueue::putBuffer(SAMPLETYPE *buffer, int size) {
    pthread_mutex_lock(&mutexBuffer);
    CyPcmBean *pcmBean = new CyPcmBean(buffer , size);
    queueBuffer.push_back(pcmBean);
    pthread_cond_signal(&condBuffer);
    pthread_mutex_unlock(&mutexBuffer);
    return 0;
}

int CyBufferQueue::getBuffer(CyPcmBean **pcmBean) {
    pthread_mutex_lock(&mutexBuffer);
    while (cyPlaystatus != NULL && !cyPlaystatus->exit){
        if (queueBuffer.size() > 0){
            *pcmBean = queueBuffer.front();
            queueBuffer.pop_front();
            break;
        } else{
            if (!cyPlaystatus->exit){
                pthread_cond_wait(&condBuffer, &mutexBuffer);
            }
        }
    }
    pthread_mutex_unlock(&mutexBuffer);
    return 0;
}

int CyBufferQueue::clearBuffer() {
    pthread_cond_signal(&condBuffer);
    pthread_mutex_lock(&mutexBuffer);
    while (!queueBuffer.empty()){
        CyPcmBean *pcmBean = queueBuffer.front();
        queueBuffer.pop_front();
        delete(pcmBean);
    }
    pthread_mutex_unlock(&mutexBuffer);
    return 0;
}

void CyBufferQueue::release() {

    noticeThread();
    clearBuffer();
    if (LOG_DEBUG){
        LOGD("CyBufferQueue  release success");
    }
}

int CyBufferQueue::noticeThread() {
    pthread_cond_signal(&condBuffer);
    return 0;
}

int CyBufferQueue::getBufferSize() {
    int size = 0;
    pthread_mutex_lock(&mutexBuffer);
    size = queueBuffer.size();
    pthread_mutex_unlock(&mutexBuffer);
    return size;
}
