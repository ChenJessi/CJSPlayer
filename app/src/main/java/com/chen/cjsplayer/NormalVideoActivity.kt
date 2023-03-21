package com.chen.cjsplayer

import android.os.Bundle
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import com.chen.cjsplayer.databinding.ActivityNormalVideoBinding

class NormalVideoActivity : AppCompatActivity() {

    var isPlaying = false
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
            //Log.d("TAG", "OnProgress time: $it")
        }
        //binding.videoPlayer.setDataSource("rtmp://mobliestream.c3tv.com:554/live/goodtv.sdp")
        //binding.videoPlayer.setDataSource("http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_4x3/gear2/prog_index.m3u8")
        //binding.videoPlayer.setDataSource("https://media.w3.org/2010/05/sintel/trailer.mp4")
        binding.videoPlayer.setDataSource("http://vfx.mtime.cn/Video/2019/03/18/mp4/190318214226685784.mp4")
        binding.btnPlay.setOnClickListener {
            Log.d("TAG", "OnClick: isPlaying  $isPlaying")
            if (isPlaying) {
                Log.d("TAG", "OnClick: stop")
                binding.videoPlayer.stop()
                isPlaying = false
            } else {
                Log.d("TAG", "OnClick: prepare")
                binding.videoPlayer.prepare()
                isPlaying = true
            }

        }

        binding.btnRelease.setOnClickListener {
            binding.videoPlayer.release()
        }


    }
}