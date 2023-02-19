package com.jessi.cjsplayer.player.base

interface IPlayerManager {

    fun setDataSource(source : String)

    fun prepare()
    fun start()
    fun stop()
    fun release()
    fun setOnPreparedListener(onPreparedListener : OnPreparedListener)

    fun setOnErrorListener(onErrorListener: OnErrorListener);
}