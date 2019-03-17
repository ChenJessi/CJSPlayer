//
// Created by CHEN on 2019/3/17.
//

#ifndef CYPLAYER_CYPCMBEAN_H
#define CYPLAYER_CYPCMBEAN_H

#include <SoundTouch.h>
using namespace soundtouch;

class CyPcmBean {
public:
    char *buffer;
    int buffersize;

public:
    CyPcmBean(SAMPLETYPE *buffer , int size);
    ~CyPcmBean();
};


#endif //CYPLAYER_CYPCMBEAN_H
