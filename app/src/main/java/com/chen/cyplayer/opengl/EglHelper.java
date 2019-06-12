package com.chen.cyplayer.opengl;

import android.nfc.cardemulation.OffHostApduService;
import android.opengl.GLSurfaceView;
import android.view.Surface;

import javax.microedition.khronos.egl.EGL;
import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;

/**
 * @author Created by CHEN on 2019/6/12
 * @email 188669@163.com
 */
public class EglHelper {
    private EGL10 mEgl;
    private EGLDisplay mEglDisplay;
    private EGLContext mEglContext;
    private EGLSurface mEglSurface;
    public void initEgl(Surface surface ,EGLContext eglContext){
        //得到Egl实例
        mEgl = (EGL10) EGLContext.getEGL();
        //得到默认的显示设备（就是窗口）
        mEglDisplay = mEgl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY);
        if (mEglDisplay == EGL10.EGL_NO_DISPLAY) {
            throw new RuntimeException("eglGetDisplay failed");
        }
        //3、初始化默认显示设备
        int[] version = new int[2];
        if(!mEgl.eglInitialize(mEglDisplay, version)) {
            throw new RuntimeException("eglInitialize failed");
        }

        //设置显示设备的属性
        int[] attrbutes = new int[]{
                EGL10.EGL_RED_SIZE, 8,
                EGL10.EGL_GREEN_SIZE,8,
                EGL10.EGL_BLUE_SIZE,8,
                EGL10.EGL_ALPHA_SIZE,8,
                EGL10.EGL_DEPTH_SIZE,8,
                EGL10.EGL_STENCIL_SIZE,8,
                EGL10.EGL_NONE
        };
        int[] num_config = new int[1];
        if (!mEgl.eglChooseConfig(mEglDisplay,attrbutes,null,1,num_config)){
            throw new IllegalArgumentException("eglChooseConfig failed");
        }
        int numConfigs = num_config[0];
        if (numConfigs <= 0 ){
            throw new IllegalArgumentException(
                    "No configs match configSpec");
        }
        //从系统中获取对应属性配置
        EGLConfig[] configs = new EGLConfig[numConfigs];
        if (!mEgl.eglChooseConfig(mEglDisplay,attrbutes, configs, numConfigs, num_config)){
            throw new IllegalArgumentException("eglChooseConfig#2 failed");
        }
        //创建EglContext
        if (eglContext != null){
            mEglContext = mEgl.eglCreateContext(mEglDisplay, configs[0], eglContext, null);
        }else {
            mEglContext = mEgl.eglCreateContext(mEglDisplay, configs[0], EGL10.EGL_NO_CONTEXT, null);
        }
        //创建渲染的Surface
        mEglSurface = mEgl.eglCreateWindowSurface(mEglDisplay,configs[0], surface , null);
        //绑定EglContext和Surface到显示设备中
        if (!mEgl.eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext)) {
            throw new RuntimeException("eglMakeCurrent fail");
        }
    }
    public boolean swapBuffers(){
        if (mEgl != null){
            return mEgl.eglSwapBuffers(mEglDisplay,mEglSurface);
        }else {
            throw new RuntimeException("egl is null");
        }
    }

    public EGLContext getmEglContext(){
        return mEglContext;
    }

    public void destoryEgl(){
        if (mEgl != null){
            mEgl.eglMakeCurrent(mEglDisplay, EGL10.EGL_NO_SURFACE,
                    EGL10.EGL_NO_SURFACE,
                    EGL10.EGL_NO_CONTEXT);
            mEgl.eglDestroySurface(mEglDisplay,mEglSurface);
            mEglSurface = null;

            mEgl.eglDestroyContext(mEglDisplay,mEglContext);
            mEglContext = null;
            mEgl.eglTerminate(mEglDisplay);
            mEglDisplay = null;
            mEgl = null;
        }
    }
}
