package com.jessi.cjsplayer.push

import android.app.Activity
import android.view.SurfaceHolder


class CJSPusher(
    activity: Activity,
    cameraId: Int,
    width: Int,
    height: Int,
    fps: Int,
    bitrate: Int
) {

    private val audioChannel by lazy { AudioChannel() }
    private val videoChannel by lazy {
        VideoChannel(
            this,
            activity,
            cameraId,
            width,
            height,
            fps,
            bitrate
        )
    }

    init {
        System.loadLibrary("cjsplayer-native-lib")

        initNative()
    }


    fun setPreviewDisplay(surfaceHolder: SurfaceHolder) {
        videoChannel.setPreviewDisplay(surfaceHolder)
    }

    fun switchCamera() {
        videoChannel.switchCamera()
    }

    fun startLive(url: String) {
        startLiveNative(url)
        audioChannel.startLive()
        videoChannel.startLive()
    }

    fun stopLive() {
        audioChannel.stopLive()
        videoChannel.stopLive()
        stopLiveNative()
    }

    fun release() {
        audioChannel.release()
        videoChannel.release()
        releaseNative()
    }



    private external fun initNative()
    private external fun startLiveNative(url: String)
    private external fun stopLiveNative()
    private external fun releaseNative()

    // 初始化音频编码器
    external fun initVideoEncoderNative(width: Int, height: Int, fps: Int, bitrate: Int)
    // 推送音频数据
    external fun pushVideoNative(data: ByteArray)


}