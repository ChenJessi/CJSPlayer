//
// Created by 哦 on 2023/2/13.
//

#ifndef CJSPLAYER_CJSPLAYER_H
#define CJSPLAYER_CJSPLAYER_H

#include <cstring>
using namespace std;

class CJSPlayer {

public:
    CJSPlayer(const char *data_source);
    ~CJSPlayer();

private:
    const char *data_source = nullptr;
};


#endif //CJSPLAYER_CJSPLAYER_H
