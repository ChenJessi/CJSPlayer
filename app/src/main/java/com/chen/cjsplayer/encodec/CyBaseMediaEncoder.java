package com.chen.cjsplayer.encodec;

import android.content.Context;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.media.MediaMuxer;
import android.view.Surface;

import com.chen.cjsplayer.log.MyLog;
import com.chen.cjsplayer.opengl.CyEGLSurfaceView;
import com.chen.cjsplayer.opengl.EglHelper;

import java.io.IOException;
import java.lang.ref.WeakReference;
import java.nio.ByteBuffer;

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

    private MediaCodec audioEncodec;
    private MediaFormat audioFormat;
    private MediaCodec.BufferInfo audioBufferinfo;

    private long audioPts = 0;
    private int sampleRate = 0;

    private MediaMuxer mediaMuxer;
    private boolean audioExit = false;
    private boolean videoExit = false;
    private boolean encodecStart = false;

    private CyEGLMediaThread cyEGLMediaThread;
    private VideoEncodecThread videoEncodecThread;
    private AudioEncodecThread audioEncodecThread;

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

    public void initEncodec(EGLContext eglContext ,String savePath, int width, int height, int sampleRate, int channelCount){
        this.eglContext = eglContext;
        this.width = width;
        this.height = height;
        initMediaEncodec(savePath, width, height, sampleRate, channelCount);
    }

    public void startRecord(){
        if (surface != null && eglContext != null){
            audioPts = 0;
            audioExit = false;
            videoExit = false;
            encodecStart = false;

            cyEGLMediaThread = new CyEGLMediaThread(new WeakReference<CyBaseMediaEncoder>(this));
            videoEncodecThread = new VideoEncodecThread(new WeakReference<CyBaseMediaEncoder>(this));
            audioEncodecThread = new AudioEncodecThread(new WeakReference<CyBaseMediaEncoder>(this));
            cyEGLMediaThread.isCreate = true;
            cyEGLMediaThread.isChange = true;
            cyEGLMediaThread.start();
            videoEncodecThread.start();
            audioEncodecThread.start();
        }
    }
    public void stopRecord(){
        if (cyEGLMediaThread != null && videoEncodecThread != null && audioEncodecThread != null){
            videoEncodecThread.exit();;
            audioEncodecThread.exit();;
            cyEGLMediaThread.onDestory();
            videoEncodecThread = null;
            cyEGLMediaThread = null;
            audioEncodecThread = null;
        }
    }


    private void initMediaEncodec(String savePath, int width, int height, int sampleRate, int channelCount) {
        try {
            mediaMuxer = new MediaMuxer(savePath,MediaMuxer.OutputFormat.MUXER_OUTPUT_MPEG_4);
            initVideoEncodec(MediaFormat.MIMETYPE_VIDEO_AVC, width , height);
            initAudioEncodec(MediaFormat.MIMETYPE_AUDIO_AAC, sampleRate, channelCount);
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

    private void initAudioEncodec(String mimeType, int sampleRate, int channelCount){
        try {
            this.sampleRate = sampleRate;
            audioBufferinfo = new MediaCodec.BufferInfo();
            audioFormat = MediaFormat.createAudioFormat(mimeType, sampleRate, channelCount);
            audioFormat.setInteger(MediaFormat.KEY_BIT_RATE, 96000);
            audioFormat.setInteger(MediaFormat.KEY_AAC_PROFILE, MediaCodecInfo.CodecProfileLevel.AACObjectLC);
            audioFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, 4096);

            audioEncodec = MediaCodec.createEncoderByType(mimeType);
            audioEncodec.configure(audioFormat, null, null
                    , MediaCodec.CONFIGURE_FLAG_ENCODE);
        } catch (IOException e) {
            e.printStackTrace();
            audioBufferinfo = null;
            audioFormat = null;
            audioFormat = null;
        }
    }

    public void putPcmData(byte[] buffer, int size){
        if (audioEncodecThread != null && !audioEncodecThread.isExit && buffer != null && size > 0){
            int inputBufferIndex = audioEncodec.dequeueInputBuffer(0);
            if (inputBufferIndex >= 0){
                ByteBuffer byteBuffer = audioEncodec.getInputBuffer(inputBufferIndex);
                byteBuffer.clear();
                byteBuffer.put(buffer);
                long pts = getAudioPts(size, sampleRate);
                audioEncodec.queueInputBuffer(inputBufferIndex, 0, size, pts, 0);
            }
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
            if(eglHelper != null) {
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
        private MediaCodec.BufferInfo videoBufferinfo;
        private MediaMuxer mediaMuxer;

        private int videoTrackIndex = -1;
        private long pts;

        public VideoEncodecThread(WeakReference<CyBaseMediaEncoder> encoder) {
            this.encoder = encoder;
            videoEncodec = encoder.get().videoEncodec;
            videoBufferinfo = encoder.get().videoBufferinfo;
            mediaMuxer = encoder.get().mediaMuxer;
            videoTrackIndex = -1;
        }

        @Override
        public void run() {
            super.run();
            pts = 0;
            videoTrackIndex = -1;
            isExit = false;
            videoEncodec.start();

            while (true){
                if(isExit) {

                    videoEncodec.stop();
                    videoEncodec.release();
                    videoEncodec = null;
                    encoder.get().videoExit = true;
                    if (encoder.get().audioExit){
                        mediaMuxer.stop();
                        mediaMuxer.release();
                        mediaMuxer = null;
                    }

                    MyLog.d("录制完成");
                    break;
                }

                int outputBufferIndex = videoEncodec.dequeueOutputBuffer(videoBufferinfo, 0);
                if (outputBufferIndex == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                    videoTrackIndex = mediaMuxer.addTrack(videoEncodec.getOutputFormat());
                    if (encoder.get().audioEncodecThread.audioTrackIndex != -1){
                        mediaMuxer.start();
                        encoder.get().encodecStart = true;
                    }

                }else {
                    while (outputBufferIndex >= 0) {
                        ByteBuffer outputBuffer = videoEncodec.getOutputBuffers()[outputBufferIndex];
                        outputBuffer.position(videoBufferinfo.offset);
                        outputBuffer.limit(videoBufferinfo.offset + videoBufferinfo.size);
                        //

                        if(pts == 0) {
                            pts = videoBufferinfo.presentationTimeUs;
                        }
                        videoBufferinfo.presentationTimeUs = videoBufferinfo.presentationTimeUs - pts;

                        mediaMuxer.writeSampleData(videoTrackIndex, outputBuffer, videoBufferinfo);
                        if(encoder.get().onMediaInfoListener != null) {
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

    private static class AudioEncodecThread extends Thread{
        private WeakReference<CyBaseMediaEncoder> encoder;
        private boolean isExit;
        private MediaCodec audioEncodec;
        private MediaCodec.BufferInfo bufferInfo;
        private MediaMuxer mediaMuxer;
        private int audioTrackIndex = -1;
        private long pts = 0;

        public AudioEncodecThread(WeakReference<CyBaseMediaEncoder> encoder) {
            this.encoder = encoder;
            audioEncodec = encoder.get().audioEncodec;
            bufferInfo = encoder.get().audioBufferinfo;
            mediaMuxer = encoder.get().mediaMuxer;
            audioTrackIndex = -1;
        }

        @Override
        public void run() {
            super.run();
            pts = 0;
            isExit = false;
            audioEncodec.start();
            while (true){
                if (isExit){
                    audioEncodec.stop();
                    audioEncodec.release();
                    audioEncodec = null;
                    encoder.get().audioExit = true;
                    if (encoder.get().videoExit){
                        mediaMuxer.stop();
                        mediaMuxer.release();;
                        mediaMuxer = null;
                    }
                    break;
                }
                int outputBufferIndex = audioEncodec.dequeueOutputBuffer(bufferInfo, 0);
                if (outputBufferIndex == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED){
                    if (mediaMuxer != null){
                        audioTrackIndex = mediaMuxer.addTrack(audioEncodec.getOutputFormat());
                        if (encoder.get().videoEncodecThread.videoTrackIndex != -1){
                            mediaMuxer.start();
                            encoder.get().encodecStart = true;
                        }
                    }
                }else {
                    while (outputBufferIndex >= 0){
                        if (encoder.get().encodecStart){
                            ByteBuffer outputBuffer = audioEncodec.getOutputBuffer(outputBufferIndex);
                            outputBuffer.position(bufferInfo.offset);
                            outputBuffer.limit(bufferInfo.offset + bufferInfo.size);
                            if (pts == 0){
                                pts = bufferInfo.presentationTimeUs;
                            }
                            bufferInfo.presentationTimeUs = bufferInfo.presentationTimeUs - pts;
                            mediaMuxer.writeSampleData(audioTrackIndex, outputBuffer, bufferInfo);
                        }
                        audioEncodec.releaseOutputBuffer(outputBufferIndex, false);
                        outputBufferIndex = audioEncodec.dequeueOutputBuffer(bufferInfo, 0);
                    }
                }
            }
        }
        public void exit(){
            isExit = true;
        }
    }


    public interface OnMediaInfoListener{
        void onMediaTime(int times);
    }

    private long getAudioPts(int size, int sampleRate){
        audioPts += (long)(1.0 * size / (sampleRate * 2 * 2) * 1000000);
        return audioPts;
    }
}
