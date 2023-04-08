package com.chen.cjsplayer

import android.content.Intent
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import com.chen.cjsplayer.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        binding.btnPlay.setOnClickListener {
            startActivity(Intent(this, NormalVideoActivity::class.java))
        }

        binding.btnPushLive.setOnClickListener {
            startActivity(Intent(this, PushLiveActivity::class.java))
        }
    }


}
