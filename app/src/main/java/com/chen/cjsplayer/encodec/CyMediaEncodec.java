package com.chen.cjsplayer.encodec;

import android.content.Context;

import com.chen.cjsplayer.push.CyEncodecPushRender;

/**
 * @author Created by CHEN on 2019/6/26
 * @email 188669@163.com
 */
public class CyMediaEncodec extends CyBaseMediaEncoder {
    private CyEncodecPushRender cyEncodecPushRender;
    public CyMediaEncodec(Context context , int textureId) {
        super(context);
        cyEncodecPushRender = new CyEncodecPushRender(context, textureId);
        setRender(cyEncodecPushRender);
        setRenderMode(CyBaseMediaEncoder.RENDERMODE_CONTINUOUSLY);
    }
}
