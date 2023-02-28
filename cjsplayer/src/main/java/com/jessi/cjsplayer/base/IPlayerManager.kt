package com.jessi.cjsplayer.base

import android.media.MediaPlayer

typealias OnPreparedListener = (()->Unit)?
typealias OnErrorListener = ((String)->Unit)?
interface IPlayerManager {

    fun setDataSource(source : String)

    fun prepare()
    fun start()
    fun stop()
    fun release()
    fun setOnPreparedListener(onPreparedListener : OnPreparedListener)

    fun setOnErrorListener(onErrorListener: OnErrorListener);
}