package com.chen.cyplayer.encodec;

import android.content.Context;

/**
 * @author Created by CHEN on 2019/6/26
 * @email 188669@163.com
 */
public class CyMediaEncodec extends CyBaseMediaEncoder {
    private CyEncodecRender cyEncodecRender;
    public CyMediaEncodec(Context context ,int textureId) {
        super(context);
        cyEncodecRender = new CyEncodecRender(context, textureId);
        setRender(cyEncodecRender);
        setRenderMode(CyBaseMediaEncoder.RENDERMODE_CONTINUOUSLY);
    }
}
