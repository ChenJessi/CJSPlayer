//
// Created by CHEN on 2019/7/15.
//

#ifndef CYPLAYER_RECORDBUFFER_H
#define CYPLAYER_RECORDBUFFER_H


class RecordBuffer {
public:
    short **buffer;
    int index = -1;

public:
    RecordBuffer(int buffersize);
    ~RecordBuffer();
    short  *getRecordBuffer();
    short * getNowBuffer();
};


#endif //CYPLAYER_RECORDBUFFER_H
