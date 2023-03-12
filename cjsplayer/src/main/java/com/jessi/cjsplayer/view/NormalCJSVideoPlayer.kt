package com.jessi.cjsplayer.view

import android.content.Context
import android.util.AttributeSet
import com.jessi.cjsplayer.R

open class NormalCJSVideoPlayer @JvmOverloads constructor(
    context: Context, attrs: AttributeSet? = null
): CJSVideoView(context, attrs) {

    init {
        initInflate()
    }

    private fun initInflate(){
        //View.inflate(context, getLayoutId(), this)
    }



    open fun getLayoutId() : Int{
        return R.layout.view_video_normal
    }
}