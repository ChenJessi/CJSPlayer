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


        binding.videoView.setOnPreparedListener {
            Log.d("TAG", "OnPrepared: 准备完成")
            binding.videoView.start()
        }
        binding.videoView.setOnErrorListener {
            Log.d("TAG", "OnError code: $it")
        }
        binding.videoView.setDataSource("http://vfx.mtime.cn/Video/2019/03/18/mp4/190318214226685784.mp4")
        binding.btnPlay.setOnClickListener {
            binding.videoView.prepare()
        }
    }
}