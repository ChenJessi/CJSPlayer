package com.chen.cyplayer.push;

import android.text.TextUtils;

/**
 * @author Created by CHEN on 2019/7/22
 * @email 188669@163.com
 */
public class CyPushVideo {
    static {
        System.loadLibrary("native-lib");
    }


    private CyConnectListenr cyConnectListenr;


    public void setCyConnectListenr(CyConnectListenr cyConnectListenr) {
        this.cyConnectListenr = cyConnectListenr;
    }

    private void onConnecting() {
        if (cyConnectListenr != null) {
            cyConnectListenr.onConnecting();
        }
    }

    private void onConnectSuccess() {
        if (cyConnectListenr != null) {
            cyConnectListenr.onConnectSuccess();
        }
    }

    private void onConnectFial(String msg) {
        if (cyConnectListenr != null) {
            cyConnectListenr.onConnectFail(msg);
        }
    }


    public void initLivePush(String url) {
        if (!TextUtils.isEmpty(url)) {
            initPush(url);
        }
    }

    public void pushSPSPPS(byte[] sps, byte[] pps){
        if (sps != null && pps != null){
            pushSPSPPS(sps, sps.length, pps, pps.length);
        }
    }

    public void pushVideoData(byte[] data, boolean keyframe){
        if (data != null){
            pushVideoData(data, data.length, keyframe);
        }
    }

    public void pushAudioData(byte[] data){
        if (data != null){
            pushAudioData(data, data.length);
        }
    }

    public void stopPush(){
        pushStop();
    }





    private native void initPush(String pushUrl);

    private native void pushSPSPPS(byte[] sps, int sps_len, byte[] pps, int pps_len);

    private native void pushVideoData(byte[] data, int data_len, boolean keyframe);

    private native void pushAudioData(byte[] data, int data_len);

    private native void pushStop();
}
