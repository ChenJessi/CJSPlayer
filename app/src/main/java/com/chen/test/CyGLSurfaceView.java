package com.chen.test;

import android.content.Context;
import android.util.AttributeSet;

import com.chen.cyplayer.opengl.CyEGLSurfaceView;

/**
 * @author Created by CHEN on 2019/6/13
 * @email 188669@163.com
 */
public class CyGLSurfaceView extends CyEGLSurfaceView {
    public CyGLSurfaceView(Context context) {
        this(context,null);
    }

    public CyGLSurfaceView(Context context, AttributeSet attrs) {
        this(context, attrs,0);
    }

    public CyGLSurfaceView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        setRender(new CyRender());
        setRenderMode(CyEGLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }
}
