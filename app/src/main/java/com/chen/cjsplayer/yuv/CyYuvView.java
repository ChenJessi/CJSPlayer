package com.chen.cjsplayer.yuv;

import android.content.Context;
import android.util.AttributeSet;

import com.chen.cjsplayer.opengl.CyEGLSurfaceView;

/**
 * @author Created by CHEN on 2019/7/10
 * @email 188669@163.com
 */
public class CyYuvView extends CyEGLSurfaceView {
    private CyYuvRender cyYuvRender;

    public CyYuvView(Context context) {
        this(context, null);
    }

    public CyYuvView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public CyYuvView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        cyYuvRender = new CyYuvRender(context);
        setRender(cyYuvRender);
        setRenderMode(CyEGLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }

    public void setFrameData(int w, int h, byte[] by, byte[] bu, byte[] bv) {
        if (cyYuvRender != null) {
            cyYuvRender.setFrameData(w, h, by, bu, bv);
            requestRender();
        }
    }
}
