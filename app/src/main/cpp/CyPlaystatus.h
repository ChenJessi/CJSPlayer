//
// Created by CHEN on 2019/3/1.
//

#ifndef CYPLAYER_CYPLAYSTATUS_H
#define CYPLAYER_CYPLAYSTATUS_H


class CyPlaystatus {
public:
    bool exit = false;
    bool load = true;
    bool seek = false;
    bool pause = false;
public:
    CyPlaystatus();
    ~CyPlaystatus();
};


#endif //CYPLAYER_CYPLAYSTATUS_H
