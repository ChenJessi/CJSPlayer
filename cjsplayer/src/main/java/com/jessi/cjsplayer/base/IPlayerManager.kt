package com.jessi.cjsplayer.base

import android.view.SurfaceView

typealias OnPreparedListener = (() -> Unit)?
typealias OnErrorListener = ((String) -> Unit)?
typealias OnProgressListener = ((Int) -> Unit)?

interface IPlayerManager {

    fun setDataSource(source: String)

    fun setSurfaceView(surfaceView: SurfaceView)
    fun prepare()
    fun start()
    fun stop()
    fun release()

    fun getDuration(): Int
    fun seek(secs: Int)


    fun setOnPreparedListener(onPreparedListener: OnPreparedListener)

    fun setOnErrorListener(onErrorListener: OnErrorListener)

    fun setOnProgressListen(onProgressListener: OnProgressListener)
}