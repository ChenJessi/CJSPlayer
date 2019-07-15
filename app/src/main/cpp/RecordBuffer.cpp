//
// Created by CHEN on 2019/7/15.
//

#include "RecordBuffer.h"

RecordBuffer::RecordBuffer(int buffersize) {
    buffer = new short *[2];
    for (int i = 0; i < 2; i++) {
        buffer[i] = new short[buffersize];
    }
}

RecordBuffer::~RecordBuffer() {
    for (int i = 0; i < 2; i++) {
        delete buffer[i];
    }
    delete buffer;
}

short *RecordBuffer::getRecordBuffer() {
    index++;
    if (index > 1){
        index = 1;
    }
    return buffer[index];
}

short *RecordBuffer::getNowBuffer() {
    return buffer[index];
}
