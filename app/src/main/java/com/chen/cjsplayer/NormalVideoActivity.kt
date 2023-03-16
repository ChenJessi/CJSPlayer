package com.chen.cjsplayer

import android.os.Bundle
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import com.chen.cjsplayer.databinding.ActivityNormalVideoBinding

class NormalVideoActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val binding = ActivityNormalVideoBinding.inflate(layoutInflater)
        setContentView(binding.root)


        binding.videoPlayer.setOnPreparedListener {
            Log.d("TAG", "OnPrepared: 准备完成 视频总时长 : ${binding.videoPlayer.getDuration()}")
            binding.videoPlayer.start()
        }
        binding.videoPlayer.setOnErrorListener {
            Log.d("TAG", "OnError code: $it")
        }
        binding.videoPlayer.setOnProgressListener {
            Log.d("TAG", "OnProgress time: $it")
        }
        //binding.videoView.setDataSource("https://media.w3.org/2010/05/sintel/trailer.mp4")
        binding.videoPlayer.setDataSource("http://vfx.mtime.cn/Video/2019/03/18/mp4/190318214226685784.mp4")
        binding.btnPlay.setOnClickListener {
            binding.videoPlayer.prepare()
        }
    }
}