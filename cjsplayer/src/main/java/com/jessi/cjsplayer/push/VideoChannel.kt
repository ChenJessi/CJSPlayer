package com.jessi.cjsplayer.push

import android.app.Activity
import android.view.SurfaceHolder
import com.jessi.cjsplayer.helper.CameraHelper

class VideoChannel(
    val pusher: CJSPusher,
    activity: Activity,
    cameraId: Int,
    width: Int,
    height: Int,
    fps: Int,
    bitrate: Int
)  {

    private var isLiving = false // 是否正在直播
    private val cameraHelper by lazy {
        CameraHelper(
            activity,
            cameraId,
            width,
            height
        )
    }

    init {
        cameraHelper.setPreviewCallback { data, camera ->
            if (isLiving) {
                pusher.pushVideoNative(data)
            }
        }
        cameraHelper.setOnChangedSizeListener { w, h ->
            pusher.initVideoEncoderNative(w, h, fps, bitrate)
        }
    }

    fun startLive() {
        isLiving = true
    }


    fun stopLive() {
        isLiving = false
    }

    fun release() {
        cameraHelper.stopPreview()
    }

    fun setPreviewDisplay(surfaceHolder: SurfaceHolder) {
        cameraHelper.setPreviewDisplay(surfaceHolder)
    }

    fun switchCamera() {
        cameraHelper.switchCamera()
    }



}