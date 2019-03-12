//
// Created by CHEN on 2019/3/1.
//

#include "CyQueue.h"

CyQueue::CyQueue(CyPlaystatus *playstatus) {
    this->playstatus = playstatus;
    pthread_mutex_init(&mutexPacket, NULL);
    pthread_cond_init(&condPacket, NULL);

}

CyQueue::~CyQueue() {
    clearAvpacket();
}

int CyQueue::putAvpacket(AVPacket *packet) {
    pthread_mutex_lock(&mutexPacket);
    queuePacket.push(packet);

    pthread_cond_signal(&condPacket);
    pthread_mutex_unlock(&mutexPacket);
    return 0;
}

int CyQueue::getAvpacket(AVPacket *packet) {

    pthread_mutex_lock(&mutexPacket);

    while (playstatus != NULL && !playstatus->exit){
        if (queuePacket.size() > 0){
            AVPacket *avPacket = queuePacket.front();
            if (av_packet_ref(packet, avPacket) == 0){
                queuePacket.pop();
            }
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            break;
        } else{
            pthread_cond_wait(&condPacket, &mutexPacket);
        }
    }
    pthread_mutex_unlock(&mutexPacket);
    return 0;
}

int CyQueue::getQueueSize() {
    int size =0;
    pthread_mutex_lock(&mutexPacket);
    size = queuePacket.size();
    pthread_mutex_unlock(&mutexPacket);
    return size;
}

void CyQueue::clearAvpacket() {
    pthread_cond_signal(&condPacket);
    pthread_mutex_unlock(&mutexPacket);

    while (!queuePacket.empty()){
        AVPacket *packet = queuePacket.front();
        queuePacket.pop();
        av_packet_free(&packet);
        av_free(packet);
        packet = NULL;
    }
    pthread_mutex_unlock(&mutexPacket);
}
