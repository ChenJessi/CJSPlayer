package com.chen.cyplayer;

import android.content.Context;
import android.util.AttributeSet;

import com.chen.cyplayer.opengl.CyEGLSurfaceView;
import com.chen.cyplayer.opengl.CyTextureRender;

/**
 * @author Created by CHEN on 2019/6/15
 * @email 188669@163.com
 */
public class CyGLTextureView extends CyEGLSurfaceView {
    public CyGLTextureView(Context context) {
        this(context,null);
    }

    public CyGLTextureView(Context context, AttributeSet attrs) {
        this(context, attrs,0);
    }

    public CyGLTextureView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        setRender(new CyTextureRender(context));
    }
}
