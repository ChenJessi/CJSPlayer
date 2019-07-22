package com.chen.cyplayer.push;

import android.text.TextUtils;

/**
 * @author Created by CHEN on 2019/7/22
 * @email 188669@163.com
 */
public class CyPushVideo {
    static {
        System.loadLibrary("native-lib");
    }

    public void initLivePush(String url)
    {
        if(!TextUtils.isEmpty(url))
        {
            initPush(url);
        }
    }

    private native void initPush(String pushUrl);
}
