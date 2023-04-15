package com.chen.cjsplayer

import android.content.Intent
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import com.chen.cjsplayer.databinding.ActivityMainBinding
import kotlin.coroutines.*

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

        cont.resume(Unit)
        Result.success(5)
    }
    val cont = suspend {
        println("Hello, World!")
    }   .createCoroutine(object : Continuation<Unit> {
        override val context: CoroutineContext
            get() = EmptyCoroutineContext

        override fun resumeWith(result: Result<Unit>) {
            result.getOrThrow()
        }
    })



}
