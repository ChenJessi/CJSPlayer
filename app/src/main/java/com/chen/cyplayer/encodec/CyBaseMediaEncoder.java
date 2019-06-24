package com.chen.cyplayer.encodec;

import android.content.Context;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.media.MediaMuxer;
import android.view.Surface;

import com.chen.cyplayer.opengl.CyEGLSurfaceView;

import java.io.IOException;

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

    public void setRender(int mRenderMode) {
        if(cyGLRender == null)
        {
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

    private void initMediaEncodec(String savePath, String mimeType, int width, int height) {
        try {
            mediaMuxer = new MediaMuxer(savePath,MediaMuxer.OutputFormat.MUXER_OUTPUT_MPEG_4);
            initVideoEncodec(mimeType, width , height);
        } catch (IOException e) {
            e.printStackTrace();
        }

    }

    private void initVideoEncodec(String mimeType, int width, int height) {
        videoBufferinfo = new MediaCodec.BufferInfo();
        videoFormat = MediaFormat.createVideoFormat(mimeType, width, height);
        videoFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);
        videoFormat.setInteger(MediaFormat.KEY_BIT_RATE, width * height *4);
        videoFormat.setInteger(MediaFormat.KEY_FRAME_RATE, 30);
    }

    private static class CyEGLMediaThread extends Thread {

    }

    private static class VideoEncodecThread extends Thread {

    }


    private interface OnMediaInfoListener{

    }
}
