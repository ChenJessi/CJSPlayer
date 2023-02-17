package com.chen.cjsplayer

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
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
            Toast.makeText(this, "准备完成", Toast.LENGTH_SHORT).show()
        }
        binding.btnPlay.setOnClickListener {
            player.prepare()
        }
    }

}
