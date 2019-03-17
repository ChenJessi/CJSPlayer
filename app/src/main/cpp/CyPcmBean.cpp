//
// Created by CHEN on 2019/3/17.
//

#include "CyPcmBean.h"

CyPcmBean::CyPcmBean(SAMPLETYPE *buffer, int size) {
 this->buffer = static_cast<char *>(malloc(size));
 this->buffersize = size;
 memcpy(this->buffer, buffer, size);
}

CyPcmBean::~CyPcmBean() {
    free(buffer);
    buffer = NULL;
}
