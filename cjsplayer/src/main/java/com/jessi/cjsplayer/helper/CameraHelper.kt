package com.jessi.cjsplayer.helper

import android.app.Activity
import android.graphics.ImageFormat
import android.hardware.Camera
import android.util.Log
import android.view.Surface
import android.view.SurfaceHolder
import kotlin.math.abs

class CameraHelper(val activity : Activity, var cameraId : Int, var width : Int, var height : Int) :
    Camera.PreviewCallback {

    private var mCamera : Camera? = null
    private var mOnChangedSizeListener : ((Int,Int)->Unit)? = null
    private var mSurfaceHolder : SurfaceHolder? = null
    fun startPreview() {
        try {
            mCamera = Camera.open(cameraId) ?: return

            val parameters = mCamera?.parameters ?: return
            parameters.previewFormat = ImageFormat.NV21
            setPreviewSize(parameters)
            setPreviewOrientation(parameters)
            mCamera?.parameters = parameters

            val buffer = ByteArray(width * height * 3 / 2)
            mCamera?.addCallbackBuffer(buffer)
            mCamera?.setPreviewCallbackWithBuffer(this)
            mCamera?.setPreviewDisplay(mSurfaceHolder)
            mOnChangedSizeListener?.invoke(width, height)
            mCamera?.startPreview()
        } catch (e: Exception) {

        }
    }

    private fun setPreviewSize(parameters: Camera.Parameters) {
        val supportedPreviewSizes = parameters.supportedPreviewSizes
        val size = supportedPreviewSizes.first()
        Log.d("CameraHelper", "support size: ${size.width} ${size.height}")
        val m = abs(size.width * size.height - width * height)
        supportedPreviewSizes.removeAt(0)
        for (i in supportedPreviewSizes.indices) {
            val s = supportedPreviewSizes[i]
            val n = abs(s.width * s.height - width * height)
            if (n < m) {
                size.width = s.width
                size.height = s.height
            }
        }
        height = size.height
        width = size.width
        parameters.setPreviewSize(width, height)
        Log.d("CameraHelper", "set size: ${size.width} ${size.height}")
    }

    private fun setPreviewOrientation(parameters: Camera.Parameters) {
        val info = Camera.CameraInfo()
        Camera.getCameraInfo(cameraId, info)
        val size = parameters.previewSize
        val mRotation = activity.windowManager.defaultDisplay.rotation
        val degrees = when (mRotation) {
            Surface.ROTATION_0 -> 0
            Surface.ROTATION_90 -> 90
            Surface.ROTATION_180 -> 180
            Surface.ROTATION_270 -> 270
            else -> 0
        }
        if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
            mCamera?.setDisplayOrientation((360 - (info.orientation + degrees) % 360) % 360)
        } else {
            mCamera?.setDisplayOrientation((info.orientation - degrees + 360) % 360)
        }

    }

    override fun onPreviewFrame(data: ByteArray?, camera: Camera?) {
        TODO("Not yet implemented")
    }
}