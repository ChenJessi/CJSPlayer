package com.chen.cyplayer.push;

/**
 * @author Created by CHEN on 2019/7/24
 * @email 188669@163.com
 *
 * push 连接监听类
 */
public interface CyConnectListenr {
    void onConnecting();

    void onConnectSuccess();

    void onConnectFail(String msg);
}
