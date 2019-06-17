package com.chen.cyplayer.opengl;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.util.AttributeSet;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import java.lang.ref.WeakReference;

import javax.microedition.khronos.egl.EGLContext;

/**
 * @author Created by CHEN on 2019/6/13
 * @email 188669@163.com
 */
public abstract class CyEGLSurfaceView extends SurfaceView implements SurfaceHolder.Callback {

    private Surface surface;
    private EGLContext eglContext;
    private CyGLRender cyGLRender;
    private CyEGLThread cyEGLThread;

    public final static int RENDERMODE_WHEN_DIRTY = 0;
    public final static int RENDERMODE_CONTINUOUSLY = 1;
    private int mRenderMode = RENDERMODE_CONTINUOUSLY;
    public CyEGLSurfaceView(Context context) {
        this(context,null);
    }

    public CyEGLSurfaceView(Context context, AttributeSet attrs) {
        this(context, attrs,0);
    }

    public CyEGLSurfaceView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        getHolder().addCallback(this);
    }

    public void setRender(CyGLRender wlGLRender) {
        this.cyGLRender = wlGLRender;
    }

    public void setRenderMode(int mRenderMode) {

        if(cyGLRender == null) {
            throw  new RuntimeException("must set render before");
        }
        this.mRenderMode = mRenderMode;
    }

    public void setSurfaceAndEglContext(Surface surface, EGLContext eglContext) {
        this.surface = surface;
        this.eglContext = eglContext;
    }

    public EGLContext getEglContext() {
        if(cyEGLThread != null)
        {
            return cyEGLThread.getEglContext();
        }
        return null;
    }

    public void requestRender() {
        if(cyEGLThread != null)
        {
            cyEGLThread.requestRender();
        }
    }
    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        if (surface == null){
            surface = holder.getSurface();
        }
        cyEGLThread = new CyEGLThread(new WeakReference<CyEGLSurfaceView>(this));
        cyEGLThread.isCreate = true;
        cyEGLThread.start();
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

        cyEGLThread.width = width;
        cyEGLThread.height = height;
        cyEGLThread.isChange = true;
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        cyEGLThread.onDestroy();
        cyEGLThread = null;
        surface = null;
        eglContext = null;
    }

     static class CyEGLThread extends Thread{
        private WeakReference<CyEGLSurfaceView> cyEGLSurfaceViewWeakReference;
        private EglHelper eglHelper = null;
        private boolean isCreate = false;
        private boolean isChange = false;
        private boolean isStart = false;
        private boolean isExit= false;
        private Object object = null;
        private int width;
        private int height;
        public CyEGLThread(WeakReference<CyEGLSurfaceView> cyEGLSurfaceViewWeakReference) {
            this.cyEGLSurfaceViewWeakReference = cyEGLSurfaceViewWeakReference;
        }

        @Override
        public void run() {
            super.run();
            isExit = false;
            isStart = false;
            object = new Object();
            eglHelper = new EglHelper();
            eglHelper.initEgl(cyEGLSurfaceViewWeakReference.get().surface, cyEGLSurfaceViewWeakReference.get().eglContext);

            while (true){
                if (isExit){
                    release();
                    break;
                }
                if (isStart){
                    if (cyEGLSurfaceViewWeakReference.get().mRenderMode == RENDERMODE_WHEN_DIRTY){
                        synchronized (object){
                            try {
                                object.wait();
                            } catch (InterruptedException e) {
                                e.printStackTrace();
                            }
                        }
                    }else if (cyEGLSurfaceViewWeakReference.get().mRenderMode == RENDERMODE_CONTINUOUSLY){
                        try {
                            Thread.sleep(1000 / 60);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    }else {
                        throw  new RuntimeException("mRenderMode is wrong value");
                    }
                }
                onCreate();
                onChanged(width,height);
                onDraw();
                isStart = true;
            }
        }

        private void onCreate(){
            if (isCreate && cyEGLSurfaceViewWeakReference.get().cyGLRender != null){
                isCreate = false;
                cyEGLSurfaceViewWeakReference.get().cyGLRender.onSurfaceCreated();
            }
        }
        private void onChanged(int width, int height){
            if (isChange && cyEGLSurfaceViewWeakReference.get().cyGLRender != null){
                isChange = false;
                cyEGLSurfaceViewWeakReference.get().cyGLRender.onSurfaceChanged(width, height);
            }
        }
        private void onDraw(){
            if (cyEGLSurfaceViewWeakReference.get().cyGLRender != null && eglHelper != null){
                cyEGLSurfaceViewWeakReference.get().cyGLRender.onDrawFrame();
                if (!isStart){
                    cyEGLSurfaceViewWeakReference.get().cyGLRender.onDrawFrame();
                }
                eglHelper.swapBuffers();
            }
        }
        private void requestRender(){
            if (object != null){
                synchronized (object){
                    object.notifyAll();
                }
            }
        };

        public void onDestroy(){
            isExit = true;
            requestRender();
        }
        public void release(){
            if (eglHelper != null){
                eglHelper.destoryEgl();
                eglHelper = null;
                object = null;
                cyEGLSurfaceViewWeakReference = null;
            }
        }
        public EGLContext getEglContext(){
            if(eglHelper != null) {
                return eglHelper.getmEglContext();
            }
            return null;
        }
    }


    public interface CyGLRender{
        void onSurfaceCreated();
        void onSurfaceChanged(int width, int height);
        void onDrawFrame();
    }
}
