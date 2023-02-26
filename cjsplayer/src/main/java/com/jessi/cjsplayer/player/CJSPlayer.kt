package com.jessi.cjsplayer.player

import android.view.SurfaceView
import com.jessi.cjsplayer.player.base.IPlayerManager
import com.jessi.cjsplayer.player.base.OnErrorListener
import com.jessi.cjsplayer.player.base.OnPreparedListener
import com.jessi.cjsplayer.player.manager.CJSPlayerManager

class CJSPlayer : IPlayerManager{

    private var playerManager : IPlayerManager? = null

    init {
        playerManager = CJSPlayerManager()
    }

    // 为了便于调试临时增加 setSurfaceView 方法
    fun setSurfaceView(surfaceView: SurfaceView){
        (playerManager as? CJSPlayerManager)?.setSurfaceView(surfaceView)
    }


    override fun setDataSource(source: String) {
        playerManager?.setDataSource(source)
    }

    override fun prepare() {
        playerManager?.prepare()
    }

    override fun start() {
        playerManager?.start()
    }

    override fun stop() {
        playerManager?.stop()
    }

    override fun release() {
        playerManager?.release()
    }

    override fun setOnPreparedListener(onPreparedListener: OnPreparedListener) {
        playerManager?.setOnPreparedListener(onPreparedListener)
    }

    override fun setOnErrorListener(onErrorListener: OnErrorListener) {
        playerManager?.setOnErrorListener(onErrorListener)
    }


}