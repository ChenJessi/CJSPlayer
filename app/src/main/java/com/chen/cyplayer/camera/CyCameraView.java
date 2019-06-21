package com.chen.cyplayer.camera;

import android.content.Context;
import android.util.AttributeSet;

import com.chen.cyplayer.opengl.CyEGLSurfaceView;

/**
 * @author Created by CHEN on 2019/6/21
 * @email 188669@163.com
 */
public class CyCameraView extends CyEGLSurfaceView {
    private CyCameraRender render;
    public CyCameraView(Context context) {
        this(context, null);
    }

    public CyCameraView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public CyCameraView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        render = new CyCameraRender();
        setRender(render);
    }
}
