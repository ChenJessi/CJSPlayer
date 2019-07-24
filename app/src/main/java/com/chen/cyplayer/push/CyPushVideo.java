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

    private native void initPush(String pushUrl);
}
