package com.jessi.cjsplayer.base

interface CJSMediaPlayerListener {

    fun onPrepared()

    fun onBufferingUpdate(time: Int)

    fun onError(code : Int)

    fun onCompletion()
}