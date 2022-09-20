package com.chen.cjsplayer.listener;

/**
 * @author Created by CHEN on 2019/3/16
 * @email 188669@163.com
 */
public interface CyOnPcmInfoListener {
    void onPcmInfo(int samplesize, byte[] buffer);
    void onPcmRate(int samplerate, int bit, int channels);
}
