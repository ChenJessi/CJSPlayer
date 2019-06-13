package com.chen.test;

import android.opengl.GLES20;

import com.chen.cyplayer.log.MyLog;
import com.chen.cyplayer.opengl.CyEGLSurfaceView;

/**
 * @author Created by CHEN on 2019/6/13
 * @email 188669@163.com
 */
public class CyRender implements CyEGLSurfaceView.CyGLRender {
    @Override
    public void onSurfaceCreated() {
        MyLog.d("onSurfaceCreated");
    }

    @Override
    public void onSurfaceChanged(int width, int height) {
        MyLog.d("onSurfaceChanged");
    }

    @Override
    public void onDrawFrame() {
        MyLog.d("onDrawFrame");
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        GLES20.glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    }
}
