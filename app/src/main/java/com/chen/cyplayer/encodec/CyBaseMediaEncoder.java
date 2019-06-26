package com.chen.cyplayer.encodec;

import android.content.Context;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.media.MediaMuxer;
import android.print.PrinterId;
import android.view.Surface;

import com.chen.cyplayer.log.MyLog;
import com.chen.cyplayer.opengl.CyEGLSurfaceView;
import com.chen.cyplayer.opengl.EglHelper;

import java.io.IOException;
import java.lang.ref.WeakReference;
import java.nio.ByteBuffer;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLContext;

/**
 * @author Created by CHEN on 2019/6/24
 * @email 188669@163.com
 */
public class CyBaseMediaEncoder {
    private Surface surface;
    private EGLContext eglContext;

    private int width;
    private int height;

    private MediaCodec videoEncodec;
    private MediaFormat videoFormat;
    private MediaCodec.BufferInfo videoBufferinfo;
    private MediaMuxer mediaMuxer;

    private CyEGLMediaThread cyEGLMediaThread;
    private VideoEncodecThread videoEncodecThread;

    private CyEGLSurfaceView.CyGLRender cyGLRender;

    public final static int RENDERMODE_WHEN_DIRTY = 0;
    public final static int RENDERMODE_CONTINUOUSLY = 1;
    private int mRenderMode = RENDERMODE_CONTINUOUSLY;

    private OnMediaInfoListener onMediaInfoListener;

    public CyBaseMediaEncoder(Context context) {

    }
    public void setRender(CyEGLSurfaceView.CyGLRender cyGLRender) {
        this.cyGLRender = cyGLRender;
    }
    public void setRenderMode(int mRenderMode) {
        if(cyGLRender == null) {
            throw  new RuntimeException("must set render before");
        }
        this.mRenderMode = mRenderMode;
    }

    public void setOnMediaInfoListener(OnMediaInfoListener onMediaInfoListener) {
        this.onMediaInfoListener = onMediaInfoListener;
    }

    public void initEncodec(EGLContext eglContext , String savePath, String mimeType, int width, int height){
        this.eglContext = eglContext;
        this.width = width;
        this.height = height;
        initMediaEncodec(savePath , mimeType , width , height);
    }

    public void startRecord(){
        if (surface != null && eglContext != null){
            cyEGLMediaThread = new CyEGLMediaThread(new WeakReference<CyBaseMediaEncoder>(this));
            videoEncodecThread = new VideoEncodecThread(new WeakReference<CyBaseMediaEncoder>(this));
            cyEGLMediaThread.isCreate = true;
            cyEGLMediaThread.isChange = true;
            cyEGLMediaThread.start();
            videoEncodecThread.start();
        }
    }
    public void stopRecord(){
        if (cyEGLMediaThread != null && videoEncodecThread != null){
            videoEncodecThread.exit();;
            cyEGLMediaThread.onDestory();
            videoEncodecThread = null;
            cyEGLMediaThread = null;
        }
    }


    private void initMediaEncodec(String savePath, String mimeType, int width, int height) {
        try {
            mediaMuxer = new MediaMuxer(savePath,MediaMuxer.OutputFormat.MUXER_OUTPUT_MPEG_4);
            initVideoEncodec(mimeType, width , height);
        } catch (IOException e) {
            e.printStackTrace();
        }

    }

    private void initVideoEncodec(String mimeType, int width, int height) {
        try {
            videoBufferinfo = new MediaCodec.BufferInfo();
            videoFormat = MediaFormat.createVideoFormat(mimeType, width, height);
            videoFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);
            videoFormat.setInteger(MediaFormat.KEY_BIT_RATE, width * height * 4);
            videoFormat.setInteger(MediaFormat.KEY_FRAME_RATE, 30);
            videoFormat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 1);

            videoEncodec = MediaCodec.createEncoderByType(mimeType);
            videoEncodec.configure(videoFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);

            surface = videoEncodec.createInputSurface();
        } catch (IOException e) {
            e.printStackTrace();
            videoEncodec = null;
            videoFormat = null;
            videoBufferinfo = null;
        }
    }

    private static class CyEGLMediaThread extends Thread {
        private WeakReference<CyBaseMediaEncoder> encoder;
        private EglHelper eglHelper;
        private Object object;

        private boolean isExit = false;
        private boolean isCreate = false;
        private boolean isChange = false;
        private boolean isStart = false;

        public CyEGLMediaThread(WeakReference<CyBaseMediaEncoder> encoder) {
            this.encoder = encoder;
        }

        @Override
        public void run() {
            super.run();
            isExit =false;
            isStart = false;
            object = new Object();
            eglHelper = new EglHelper();
            eglHelper.initEgl(encoder.get().surface, encoder.get().eglContext);

            while (true){
                if (isExit){
                    release();
                    break;
                }

                if (isStart){
                    if (encoder.get().mRenderMode == RENDERMODE_WHEN_DIRTY){
                        synchronized (object){
                            try {
                                object.wait();
                            } catch (InterruptedException e) {
                                e.printStackTrace();
                            }
                        }
                    }else if (encoder.get().mRenderMode == RENDERMODE_CONTINUOUSLY){
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
                onChange(encoder.get().width, encoder.get().height);
                onDraw();
                isStart = true;
            }
        }
        private void onCreate() {
            if(isCreate && encoder.get().cyGLRender != null) {
                isCreate = false;
                encoder.get().cyGLRender.onSurfaceCreated();
            }
        }

        private void onChange(int width, int height) {
            if(isChange && encoder.get().cyGLRender != null) {
                isChange = false;
                encoder.get().cyGLRender.onSurfaceChanged(width, height);
            }
        }

        private void onDraw() {
            if(encoder.get().cyGLRender != null && eglHelper != null) {
                encoder.get().cyGLRender.onDrawFrame();
                if(!isStart) {
                    encoder.get().cyGLRender.onDrawFrame();
                }
                eglHelper.swapBuffers();

            }
        }

        public void release(){
            if (eglHelper != null){
                eglHelper.destoryEgl();
                eglHelper = null;
                object = null;
                encoder = null;
            }
        }
        private void requestRender()
        {
            if(object != null) {
                synchronized (object) {
                    object.notifyAll();
                }
            }
        }

        public void onDestory() {
            isExit = true;
            requestRender();
        }


    }

    private static class VideoEncodecThread extends Thread {
        private WeakReference<CyBaseMediaEncoder> encoder;

        private boolean isExit;

        private MediaCodec videoEncodec;
        private MediaFormat videoFormat;
        private MediaCodec.BufferInfo videoBufferinfo;
        private MediaMuxer mediaMuxer;

        private int videoTrackIndex;
        private long pts;

        public VideoEncodecThread(WeakReference<CyBaseMediaEncoder> encoder) {
            this.encoder = encoder;
            videoEncodec = encoder.get().videoEncodec;
            videoFormat = encoder.get().videoFormat;
            videoBufferinfo = encoder.get().videoBufferinfo;
            mediaMuxer = encoder.get().mediaMuxer;
        }

        @Override
        public void run() {
            super.run();
            pts = 0;
            videoTrackIndex = -1;
            isExit = false ;
            videoEncodec.start();

            while (true){
                if (isExit){
                    videoEncodec.stop();
                    videoEncodec.release();
                    videoEncodec = null;
                    MyLog.d("录制完成");
                    break;
                }

                int outputBufferIndex = videoEncodec.dequeueOutputBuffer(videoBufferinfo, 0);
                if (outputBufferIndex == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                    videoTrackIndex = mediaMuxer.addTrack(videoFormat);
                    mediaMuxer.start();
                }else {
                    while (outputBufferIndex >= 0){
                        ByteBuffer outputBuffer = videoEncodec.getOutputBuffer(outputBufferIndex);
                        outputBuffer.position(videoBufferinfo.offset);
                        outputBuffer.limit(videoBufferinfo.offset + videoBufferinfo.size);

                        if (pts == 0) {
                            pts = videoBufferinfo.presentationTimeUs;
                        }
                        videoBufferinfo.presentationTimeUs = videoBufferinfo.presentationTimeUs - pts;

                        mediaMuxer.writeSampleData(videoTrackIndex, outputBuffer, videoBufferinfo);
                        if (encoder.get().onMediaInfoListener != null){
                            encoder.get().onMediaInfoListener.onMediaTime((int) (videoBufferinfo.presentationTimeUs / 1000000));
                        }
                        videoEncodec.releaseOutputBuffer(outputBufferIndex, false);
                        outputBufferIndex = videoEncodec.dequeueOutputBuffer(videoBufferinfo, 0);
                    }
                }

            }
        }
        public void exit() {
            isExit = true;
        }
    }


    public interface OnMediaInfoListener{
        void onMediaTime(int times);
    }
}
