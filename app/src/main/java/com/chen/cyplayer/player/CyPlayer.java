package com.chen.cyplayer.player;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.os.Handler;
import android.os.Message;
import android.text.TextUtils;
import android.util.Log;
import android.view.Surface;

import com.chen.cyplayer.enums.MuteEnum;
import com.chen.cyplayer.bean.CyTimeInfoBean;
import com.chen.cyplayer.listener.CyOnCompleteListener;
import com.chen.cyplayer.listener.CyOnErrorListener;
import com.chen.cyplayer.listener.CyOnLoadListener;
import com.chen.cyplayer.listener.CyOnPreparedListener;
import com.chen.cyplayer.listener.CyOnPauseResumeListener;
import com.chen.cyplayer.listener.CyOnPcmInfoListener;
import com.chen.cyplayer.listener.CyOnRecordTimeListener;
import com.chen.cyplayer.listener.CyOnTimeInfoListener;
import com.chen.cyplayer.listener.CyOnValumeDBListener;
import com.chen.cyplayer.log.MyLog;
import com.chen.cyplayer.opengl.CyGLSurfaceView;
import com.chen.cyplayer.util.CyVideoSupportUitl;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * @author Created by CHEN on 2019/2/26
 * @email 188669@163.com
 */
public class CyPlayer {
    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("avcodec");
        System.loadLibrary("avdevice");
        System.loadLibrary("avfilter");
        System.loadLibrary("avformat");
        System.loadLibrary("avutil");
        System.loadLibrary("postproc");
        System.loadLibrary("swresample");
        System.loadLibrary("swscale");
    }


    private  final int MSG_TIMEINFO = 1000;

    private String source;//数据源

    private CyTimeInfoBean timeInfoBean = null;
    private boolean playNext = false;
    private int duration = -1;
    private int volumePercent = 50;
    private  MuteEnum muteEnum = MuteEnum.MUTE_CENTER;
    private float pitch = 1.0f;
    private float speed = 1.0f;
    private boolean initmediacodec = false;
    private boolean isCut = false;

    private CyOnPreparedListener cyOnPreparedListener;
    private CyOnLoadListener cyOnLoadListener;
    private CyOnPauseResumeListener cyOnPauseResumeListener;
    private CyOnTimeInfoListener cyOnTimeInfoListener;
    private CyOnErrorListener cyOnErrorListener;
    private CyOnCompleteListener cyOnCompleteListener;
    private CyOnValumeDBListener cyOnValumeDBListener;
    private CyOnRecordTimeListener cyOnRecordTimeListener;
    private CyOnPcmInfoListener cyOnPcmInfoListener;

    private CyGLSurfaceView cyGLSurfaceView;

    private CyPlayer() {}

    private static class SingletonInstance {
        private static final CyPlayer instance = new CyPlayer();
    }
    public static CyPlayer getInstance(){
        return SingletonInstance.instance;
    }
    /**
     * 设置数据源
     * @param source
     */
    public void setSource(String source) {
        this.source = source;
    }

    /**
     * 设置准备接口回调
     * @param cyOnPreparedListener
     */
    public void setCyOnPreparedListener(CyOnPreparedListener cyOnPreparedListener) {
        this.cyOnPreparedListener = cyOnPreparedListener;
    }

    public void setCyOnLoadListener(CyOnLoadListener cyOnLoadListener) {
        this.cyOnLoadListener = cyOnLoadListener;
    }

    public void setCyOnPauseResumeListener(CyOnPauseResumeListener cyOnPauseResumeListener) {
        this.cyOnPauseResumeListener = cyOnPauseResumeListener;
    }

    public void setCyOnTimeInfoListener(CyOnTimeInfoListener cyOnTimeInfoListener) {
        this.cyOnTimeInfoListener = cyOnTimeInfoListener;
    }

    public void setCyOnErrorListener(CyOnErrorListener cyOnErrorListener) {
        this.cyOnErrorListener = cyOnErrorListener;
    }

    public void setCyOnCompleteListener(CyOnCompleteListener cyOnCompleteListener) {
        this.cyOnCompleteListener = cyOnCompleteListener;
    }

    public void setCyOnValumeDBListener(CyOnValumeDBListener cyOnValumeDBListener) {
        this.cyOnValumeDBListener = cyOnValumeDBListener;
    }

    public void setCyOnRecordTimeListener(CyOnRecordTimeListener cyOnRecordTimeListener) {
        this.cyOnRecordTimeListener = cyOnRecordTimeListener;
    }

    public void setCyOnPcmInfoListener(CyOnPcmInfoListener cyOnPcmInfoListener) {
        this.cyOnPcmInfoListener = cyOnPcmInfoListener;
    }

    public void setCyGLSurfaceView(CyGLSurfaceView cyGLSurfaceView) {
        this.cyGLSurfaceView = cyGLSurfaceView;
    }

    public void prepared(){
        if (TextUtils.isEmpty(source)){
            MyLog.d("source not be empty!");
            return;
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_prepared(source);
            }
        }).start();
    }

    public void start(){
        if (TextUtils.isEmpty(source)){
            MyLog.d("source is empty!");
            return;
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                setVolume(volumePercent);
                setMute(muteEnum);
                setPitch(pitch);
                setSpeed(speed);
                n_start();
            }
        }).start();
    }

    public void pause(){
        n_pause();
        if (cyOnPauseResumeListener != null){
            cyOnPauseResumeListener.onPause(true);
        }
    }

    public void resume(){
        n_resume();
        if (cyOnPauseResumeListener != null){
            cyOnPauseResumeListener.onPause(false);
        }
    }
    public void stop(){
        timeInfoBean = null;
        duration = -1;
        stopRecord();
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_stop();
            }
        }).start();
    }

    public void seek(int secds){
        n_seek(secds);
    }
    public void playNext(String url) {
        source = url;
        playNext = true;
        stop();
    }

    public int getDuration(){
        if (duration < 0){
            duration = n_duration();
        }
        return duration;
    }

    public void setVolume(int percent){
        if (percent >= 0 && percent <= 100){
            volumePercent = percent;
            n_volume(percent);
        }
    }

    public void setMute(MuteEnum mute){
        muteEnum = mute;
        n_mute(mute.getValue());
    }

    public int getVolumePercent(){
        return volumePercent;
    }

    public void setPitch(float pitch){
        this.pitch = pitch;
        n_pitch(pitch);
    }

    public void setSpeed(float speed){
        this.speed = speed;
        n_speed(speed);
    }

    public void startRecord(File outfile){
        if (!initmediacodec){
            audioSamplerate = n_samplerate();
            if (audioSamplerate > 0){
                initmediacodec = true;
                initMediacodec(audioSamplerate, outfile );
                n_startstoprecord(true);
                MyLog.d("开始录制");
            }
        }
    }


    public void resumeRcord()
    {
        n_startstoprecord(true);
        MyLog.d("继续录制");
    }
    public void stopRecord(){
        if (initmediacodec){
            n_startstoprecord(false);
            releaseMedicacodec();
        }
    }
    public void pauseRecord(){
        n_startstoprecord(false);
    }

    public void resumeRecord(){
        n_startstoprecord(true);
    }

    public void cutAudioPlay(int start_time, int end_time){
        boolean showPcm = cyOnPcmInfoListener != null;
        if (n_cutaudioplay(start_time, end_time, showPcm)){
            start();
        }else {
            stop();
            onCallError(2001, "cutaudio params is wrong");
        }
    }
    /**
     * c++回调java的方法
     */
    private void onCallPrepared(){
        if (cyOnPreparedListener != null){
            cyOnPreparedListener.onPrepared();
        }
    }

    private void onCallLoad(boolean load){
        if (cyOnLoadListener != null){
            cyOnLoadListener.onLoad(load);
        }
    }

    private void onCallTimeInfo(int currentTime, int totalTime){
        if (cyOnTimeInfoListener != null){
            if (timeInfoBean == null){
                timeInfoBean = new CyTimeInfoBean();
            }
            timeInfoBean.setCurrentTime(currentTime);
            timeInfoBean.setTotalTime(totalTime);

            Message message = new Message();
            message.what = MSG_TIMEINFO;
            message.obj = timeInfoBean;
            handler.sendMessage(message);
        }
    }

    private void onCallError(int code, String msg){
        stop();
        if (cyOnErrorListener != null){
            cyOnErrorListener.onError(code, msg);
        }
    }

    private void onCallComplete(){
        stop();
        if (cyOnCompleteListener != null){
            cyOnCompleteListener.onComplete();
        }
    }

    private void onCallNext(){
        if (playNext){
            playNext = false;
            prepared();
        }
    }


    private void onCallValumeDB(int db){
        if (cyOnValumeDBListener != null){
            cyOnValumeDBListener.onDbValue(db);
        }
    }

    private void onCallPcmInfo(int samplesize, byte[] buffer){
        if (cyOnPcmInfoListener != null){
            cyOnPcmInfoListener.onPcmInfo(samplesize, buffer);
        }
    }

    private void onCallPcmRate(int samplerate){
        if (cyOnPcmInfoListener != null){
            cyOnPcmInfoListener.onPcmRate(samplerate, 16, 2);
        }
    }
    private void onCallRenderYUV(int width, int height, byte[] y, byte[] u, byte[] v) {
        MyLog.d("获取到视频的yuv数据");
        if (cyGLSurfaceView != null){
            cyGLSurfaceView.setYUVData(width, height, y, u, v);
        }
    }
    private boolean onCallIsSupportMediaCodec(String ffcodename){
        return CyVideoSupportUitl.isSupportCodec(ffcodename);
    };

    private void decodeAVPacket(int datasize, byte[] data){
        if (surface != null && datasize > 0 && data != null){
            int intputBufferIndex = mediaCodec.dequeueInputBuffer(10);
            if (intputBufferIndex >= 0){
                ByteBuffer byteBuffer = mediaCodec.getOutputBuffer(intputBufferIndex);
                byteBuffer.clear();
                byteBuffer.put(data);
                mediaCodec.queueInputBuffer(intputBufferIndex,0, datasize,0, 0);
                int outputBufferIndex = mediaCodec.dequeueOutputBuffer(info , 10);
                while (outputBufferIndex >= 0 ){
                    mediaCodec.releaseOutputBuffer(outputBufferIndex,true);
                    outputBufferIndex = mediaCodec.dequeueOutputBuffer(info, 10);
                }
            }
        }
    }
    private Handler handler = new Handler(){
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what){
                case MSG_TIMEINFO:
                    CyTimeInfoBean cyTimeInfoBean = (CyTimeInfoBean) msg.obj;
                    if (cyTimeInfoBean == null){
                        MyLog.d("info error : "+ cyTimeInfoBean);
                    }
                    cyOnTimeInfoListener.timeInfo(cyTimeInfoBean);
                    break;
            }
        }
    };

    private native void n_prepared(String source);
    private native void n_start();
    private native int n_duration();
    private native void n_pause();
    private native void n_resume();
    private native void n_stop();
    private native void n_seek(int secds);
    private native void n_volume(int percent);
    private native void n_mute(int mute);
    private native void n_pitch(float pitch);
    private native void n_speed(float speed);
    private native int n_samplerate();
    private native void n_startstoprecord(boolean start);
    private native boolean n_cutaudioplay(int start_time, int end_time ,boolean showPcm);


    /**
     *  mediacodec
     */
    private MediaFormat encoderFormat = null;
    private MediaCodec encoder = null;
    private MediaCodec.BufferInfo audioInfo = null;
    private FileOutputStream outputStream = null;
    private int perpcmsize = 0;
    private int aacsamplerate = 4;
    private byte[] outByteBuffer = null;
    private double recordTime = 0;
    private int audioSamplerate = 0;
    private Surface surface;

    private MediaFormat mediaFormat = null;
    private MediaCodec mediaCodec = null;
    private MediaCodec.BufferInfo info = null;
    /**
     *
     * @param codecName
     * @param width
     * @param height
     * @param csd_0
     * @param csd_1
     */
    private void initMediaCodec(String codecName, int width, int height, byte[] csd_0, byte[] csd_1 ){
        if (surface != null){
            try {
                String mime = CyVideoSupportUitl.findCodecName(codecName);
                mediaFormat = MediaFormat.createVideoFormat(mime,width,height);
                mediaFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, width * height);
                mediaFormat.setByteBuffer("csd-0", ByteBuffer.wrap(csd_0));
                mediaFormat.setByteBuffer("csd-1", ByteBuffer.wrap(csd_1));
                MyLog.d(mediaFormat.toString());

                mediaCodec = MediaCodec.createDecoderByType(mime);
            }catch (Exception e){
                e.printStackTrace();
            }
        }else {
            if(cyOnErrorListener != null){
                cyOnErrorListener.onError(2001, "surface is null");
            }
        }
    }





    private void initMediacodec(int samperate, File outfile){
        try {
            aacsamplerate = getADTSsamplerate(samperate);
            encoderFormat = MediaFormat.createAudioFormat(MediaFormat.MIMETYPE_AUDIO_AAC, samperate, 2);
            encoderFormat.setInteger(MediaFormat.KEY_BIT_RATE, 96000);
            encoderFormat.setInteger(MediaFormat.KEY_AAC_PROFILE, MediaCodecInfo.CodecProfileLevel.AACObjectLC);
            encoderFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, 4096);
            encoder = MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_AUDIO_AAC);
            audioInfo = new MediaCodec.BufferInfo();
            if(encoder == null) {
                MyLog.d("craete encoder wrong");
                return;
            }
            recordTime = 0;
            encoder.configure(encoderFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
            outputStream = new FileOutputStream(outfile);
            encoder.start();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void encodecPcmToAAc(int size, byte[] buffer){
        if (buffer != null && encoder != null){
            recordTime += size * 1.0 / (audioSamplerate * 2 * (16 / 8));
            if (cyOnRecordTimeListener != null){
                cyOnRecordTimeListener.onRecordTime((int)recordTime);
            }
            int inputBufferindex = encoder.dequeueInputBuffer(0);
            if(inputBufferindex >= 0) {
                ByteBuffer byteBuffer = encoder.getInputBuffer(inputBufferindex);
                byteBuffer.clear();
                byteBuffer.put(buffer);
                encoder.queueInputBuffer(inputBufferindex, 0, size, 0, 0);
            }
            int index = encoder.dequeueOutputBuffer(audioInfo, 0);

            while (index >= 0){
                try {
                    perpcmsize = audioInfo.size + 7;
                    outByteBuffer = new byte[perpcmsize];

                    ByteBuffer byteBuffer = encoder.getOutputBuffer(index);
                    byteBuffer.position(audioInfo.offset);
                    byteBuffer.limit(audioInfo.offset + audioInfo.size);

                    addADtsHeader(outByteBuffer, perpcmsize, aacsamplerate);

                    byteBuffer.get(outByteBuffer, 7, audioInfo.size);
                    byteBuffer.position(audioInfo.offset);
                    outputStream.write(outByteBuffer, 0, perpcmsize);

                    encoder.releaseOutputBuffer(index, false);
                    index = encoder.dequeueOutputBuffer(audioInfo, 0);
                    outByteBuffer = null;
                    MyLog.d("编码...");
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private void addADtsHeader(byte[] packet, int packetLen, int samplerate){
        int profile = 2; // AAC LC
        int freqIdx = samplerate; // samplerate
        int chanCfg = 2; // CPE

        packet[0] = (byte) 0xFF; // 0xFFF(12bit) 这里只取了8位，所以还差4位放到下一个里面
        packet[1] = (byte) 0xF9; // 第一个t位放F
        packet[2] = (byte) (((profile - 1) << 6) + (freqIdx << 2) + (chanCfg >> 2));
        packet[3] = (byte) (((chanCfg & 3) << 6) + (packetLen >> 11));
        packet[4] = (byte) ((packetLen & 0x7FF) >> 3);
        packet[5] = (byte) (((packetLen & 7) << 5) + 0x1F);
        packet[6] = (byte) 0xFC;
    }

    private int getADTSsamplerate(int samplerate){
        int rate = 4;
        switch (samplerate) {
            case 96000:
                rate = 0;
                break;
            case 88200:
                rate = 1;
                break;
            case 64000:
                rate = 2;
                break;
            case 48000:
                rate = 3;
                break;
            case 44100:
                rate = 4;
                break;
            case 32000:
                rate = 5;
                break;
            case 24000:
                rate = 6;
                break;
            case 22050:
                rate = 7;
                break;
            case 16000:
                rate = 8;
                break;
            case 12000:
                rate = 9;
                break;
            case 11025:
                rate = 10;
                break;
            case 8000:
                rate = 11;
                break;
            case 7350:
                rate = 12;
                break;
        }
        return rate;
    }

    private void releaseMedicacodec()
    {
        if(encoder == null) {
            return;
        }
        try {
            outputStream.close();
            outputStream = null;
            encoder.stop();
            encoder.release();
            encoder = null;
            encoderFormat = null;
            info = null;
            initmediacodec = false;

            MyLog.d("录制完成...");
        } catch (IOException e) {
            e.printStackTrace();
        }
        finally {
            if(outputStream != null)
            {
                try {
                    outputStream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
                outputStream = null;
            }
        }
    }
}
