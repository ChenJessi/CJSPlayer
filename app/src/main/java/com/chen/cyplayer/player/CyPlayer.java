package com.chen.cyplayer.player;

import android.os.Handler;
import android.os.Message;
import android.text.TextUtils;
import android.util.Log;

import com.chen.cyplayer.enums.MuteEnum;
import com.chen.cyplayer.bean.CyTimeInfoBean;
import com.chen.cyplayer.listener.CyOnCompleteListener;
import com.chen.cyplayer.listener.CyOnErrorListener;
import com.chen.cyplayer.listener.CyOnLoadListener;
import com.chen.cyplayer.listener.CyOnParparedListener;
import com.chen.cyplayer.listener.CyOnPauseResumeListener;
import com.chen.cyplayer.listener.CyOnTimeInfoListener;
import com.chen.cyplayer.log.MyLog;

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

    private CyOnParparedListener cyOnParparedListener;
    private CyOnLoadListener cyOnLoadListener;
    private CyOnPauseResumeListener cyOnPauseResumeListener;
    private CyOnTimeInfoListener cyOnTimeInfoListener;
    private CyOnErrorListener cyOnErrorListener;
    private CyOnCompleteListener cyOnCompleteListener;

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
     * @param cyOnParparedListener
     */
    public void setCyOnParparedListener(CyOnParparedListener cyOnParparedListener) {
        this.cyOnParparedListener = cyOnParparedListener;
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

    public void parpared(){
        if (TextUtils.isEmpty(source)){
            MyLog.d("source not be empty!");
            return;
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_parpared(source);
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

    /**
     * c++回调java的方法
     */
    public void onCallParpared(){
        if (cyOnParparedListener != null){
            cyOnParparedListener.onParpared();
        }
    }

    public void onCallLoad(boolean load){
        if (cyOnLoadListener != null){
            cyOnLoadListener.onLoad(load);
        }
    }

    public void onCallTimeInfo(int currentTime, int totalTime){
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

    public void onCallError(int code, String msg){
        stop();
        if (cyOnErrorListener != null){
            cyOnErrorListener.onError(code, msg);
        }
    }

    public void onCallComplete(){
        stop();
        if (cyOnCompleteListener != null){
            cyOnCompleteListener.onComplete();
        }
    }

    public void onCallNext(){
        if (playNext){
            playNext = false;
            parpared();
        }
    }


    Handler handler = new Handler(){
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

    private native void n_parpared(String source);
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

}
