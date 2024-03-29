package com.jessi.cjsplayer.view

import android.content.Context
import android.util.AttributeSet
import android.view.View
import android.widget.SeekBar
import android.widget.TextView
import com.chen.cjsplayer.secondToString
import com.jessi.cjsplayer.R

open class NormalCJSVideoPlayer @JvmOverloads constructor(
    context: Context, attrs: AttributeSet? = null
): CJSVideoView(context, attrs) {

    private val tvTime : TextView by lazy { findViewById(R.id.tvTime) }
    private val seekBar : SeekBar by lazy { findViewById(R.id.seekBar) }

    private var duration = 0

    private var isTouch = false;
    init {
        initInflate()
        initViewListener()
    }

    private fun initInflate(){
        View.inflate(context, getLayoutId(), this)
    }

    open fun getLayoutId() : Int{
        return R.layout.view_video_normal
    }


    private fun initViewListener(){

        seekBar.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener{
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                if (fromUser){
                    post {
                        tvTime.text = if(duration >= 3600){
                            "${progress.secondToString()}/${duration.secondToString()}"
                        }else {
                            "${progress.secondToString()}/${duration.secondToString()}"
                        }
                    }
                }
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {
                isTouch = true
            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {
                isTouch = false
               val time = this@NormalCJSVideoPlayer.seekBar.progress
               seek(time)
            }
        })
    }


    override fun onPrepared() {
        super.onPrepared()
        post {
            duration = getDuration()
            seekBar.max = duration
            tvTime.text = if(duration >= 3600){
                "00:00:00/${duration.secondToString()}"
            }else {
                "00:00/${duration.secondToString()}"
            }
        }
    }

    override fun onBufferingUpdate(time: Int) {
        super.onBufferingUpdate(time)
        if (!isTouch){
            if(seekBar.progress == time){
                return
            }
            post {
                duration = if (duration == 0) getDuration() else duration
                val strProgress = "${time.secondToString()}/${duration.secondToString()}"
                tvTime.text = strProgress
                seekBar.progress = time
            }
        }
    }

}