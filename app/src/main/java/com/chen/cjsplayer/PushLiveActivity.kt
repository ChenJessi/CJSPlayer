package com.chen.cjsplayer

import android.hardware.Camera
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.chen.cjsplayer.databinding.ActivityPushLiveBinding
import com.jessi.cjsplayer.push.CJSPusher

class PushLiveActivity : AppCompatActivity() {

    private val livePush by lazy {
        // 前置摄像头，宽，高，fps(每秒25帧)，码率/比特率：https://blog.51cto.com/u_7335580/2058648
        CJSPusher(this, Camera.CameraInfo.CAMERA_FACING_FRONT, 640, 480, 25, 800000)
    }
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val binding = ActivityPushLiveBinding.inflate(layoutInflater)
        setContentView(binding.root)


        livePush.setPreviewDisplay(binding.surfaceView.holder)
        binding.btnStart.setOnClickListener {

            livePush.startLive("rtmp://sendtc3.douyu.com/live/6441662rypf6lvhO?wsSecret=316193e17184bb72085df8d7fbc13aa2&wsTime=643aae5d&wsSeek=off&wm=0&tw=0&roirecognition=0&record=flv&origin=tct&txHost=sendtc3a.douyu.com")
        }
        binding.btnStop.setOnClickListener {
            livePush.stopLive()
        }
        binding.btnSwitchCamera.setOnClickListener {
            livePush.switchCamera()
        }

    }

    override fun onDestroy() {
        super.onDestroy()
        livePush.release()
    }
}