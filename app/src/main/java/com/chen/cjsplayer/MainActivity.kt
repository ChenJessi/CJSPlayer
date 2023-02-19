package com.chen.cjsplayer

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import android.widget.Toast
import com.chen.cjsplayer.databinding.ActivityMainBinding
import com.jessi.cjsplayer.player.CJSPlayer
import kotlin.math.max

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)


        val player = CJSPlayer()
        player.setOnPreparedListener {
            Log.d("TAG", "OnPrepared: 准备完成")
            player.start()
        }
        player.setOnErrorListener {
            Log.d("TAG", "OnError code: $it")
        }
        player.setDataSource("http://vfx.mtime.cn/Video/2019/03/18/mp4/190318214226685784.mp4")
        binding.btnPlay.setOnClickListener {
            player.prepare()
        }
    }

}
