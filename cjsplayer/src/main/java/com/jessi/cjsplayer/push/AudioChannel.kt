package com.jessi.cjsplayer.push

import android.annotation.SuppressLint
import android.media.AudioFormat
import android.media.AudioRecord
import android.media.MediaRecorder
import androidx.core.content.PermissionChecker.*
import kotlinx.coroutines.*
import kotlin.math.max


@SuppressLint("MissingPermission")
class AudioChannel(val pusher: CJSPusher): CoroutineScope by MainScope() {


    private var isLiving = false // 是否正在直播
    private val channels  = 2   // 通道数
    private val channelConfig by lazy {
        if (channels == 2) {
            AudioFormat.CHANNEL_IN_STEREO
        } else {
            AudioFormat.CHANNEL_IN_MONO
        }
    }
    // 单通道样本数1024 * 通道数2 = 2048 * 2(位深，一个样本16bit，2字节) = 4096
    private val inputSamples by lazy {
        pusher.getInputSamples() * 2
    }

    private val audioRecord by lazy {

        // 获取最小缓冲区
        val minBufferSize = AudioRecord.getMinBufferSize(
            44100,
            channelConfig,
            AudioFormat.ENCODING_PCM_16BIT
        ) * 2

         AudioRecord(
            MediaRecorder.AudioSource.MIC, // 音频源 麦克风
            44100,  // 采样率
            channelConfig,
            AudioFormat.ENCODING_PCM_16BIT, // 采样位深
            max(minBufferSize, inputSamples)    // 缓冲区大小
        )
    }

    init {
        // 初始化音频编码器
        pusher.initAudioEncoderNative(44100, channels)

    }



    fun startLive() {
        isLiving = true
        launch(Dispatchers.IO) {
            audioRecord.startRecording()

            val buffer = ByteArray(inputSamples)// 缓冲大小
            while (isLiving) {
                val len = audioRecord.read(buffer, 0, buffer.size)
                if (len > 0) {
                    pusher.pushAudioNative(buffer)
                }
            }
            audioRecord.stop()
        }
    }

    fun stopLive() {
        isLiving = false
    }

    fun release() {
        audioRecord.release()
    }
}