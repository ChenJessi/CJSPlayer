package com.chen.cyplayer.push;

import android.content.Context;

import com.chen.cyplayer.encodec.CyBaseMediaEncoder;
import com.chen.cyplayer.encodec.CyEncodecRender;

/**
 * @author Created by CHEN on 2019/6/26
 * @email 188669@163.com
 */
public class CyPushEncodec extends CyBasePushEncoder {
    private CyEncodecPushRender cyEncodecRender;
    public CyPushEncodec(Context context , int textureId) {
        super(context);
        cyEncodecRender = new CyEncodecPushRender(context, textureId);
        setRender(cyEncodecRender);
        setRenderMode(CyBasePushEncoder.RENDERMODE_CONTINUOUSLY);
    }
}
