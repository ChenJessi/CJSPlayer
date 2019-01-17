package com.chen.ffmpegPlayer.Audio;

/**
 * @author Created by CHEN on 2019/1/15
 * @email 188669@163.com
 * 音频播放器
 */
public class AudioPlayer {
//    static {
//        System.loadLibrary("CXAudio");
//    }

    public static native void playPCM(String url);

}
