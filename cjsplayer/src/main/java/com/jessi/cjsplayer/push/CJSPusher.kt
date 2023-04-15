package com.jessi.cjsplayer.push

import android.Manifest
import android.app.Activity
import android.content.pm.PackageManager
import android.view.SurfaceHolder
import android.widget.Toast
import androidx.core.app.ActivityCompat


class CJSPusher(
    val activity: Activity,
    cameraId: Int,
    width: Int,
    height: Int,
    fps: Int,
    bitrate: Int
) {

    private val audioChannel by lazy { AudioChannel(this) }
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
        if (ActivityCompat.checkSelfPermission(
                activity,
                Manifest.permission.RECORD_AUDIO
            ) != PackageManager.PERMISSION_GRANTED
        ) {
            Toast.makeText(activity, "没有录音权限", Toast.LENGTH_SHORT).show()
            return
        }

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


    fun getInputSamples(): Int {
        return getInputSamplesNative()
    }



    private external fun initNative()
    private external fun startLiveNative(url: String)
    private external fun stopLiveNative()
    private external fun releaseNative()

    // 初始化音频编码器
    external fun initVideoEncoderNative(width: Int, height: Int, fps: Int, bitrate: Int)
    // 推送音频数据
    external fun pushVideoNative(data: ByteArray)


    // 初始化音频编码器
    external fun initAudioEncoderNative(sampleRateInHz: Int, channel: Int)

    // 获取 facc 编码器样本数
    private external fun getInputSamplesNative(): Int

    // 推送音频数据
    external fun pushAudioNative(data: ByteArray)
}