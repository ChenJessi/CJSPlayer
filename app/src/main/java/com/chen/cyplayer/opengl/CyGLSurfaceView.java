package com.chen.cyplayer.opengl;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

/**
 * @author Created by CHEN on 2019/3/26
 * @email 188669@163.com
 */
public class CyGLSurfaceView extends GLSurfaceView {

    private CyRender cyRender;

    public CyGLSurfaceView(Context context) {
        this(context,null);
    }

    public CyGLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setEGLContextClientVersion(2);
        cyRender = new CyRender(context);
        setRenderer(cyRender);
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }

    public void setYUVData(int width , int height , byte[] y, byte[] u, byte[] v){
        if (cyRender != null){
            cyRender.setYUVRenderData(width, height, y, u, v);
            requestRender();
        }
    }
}
