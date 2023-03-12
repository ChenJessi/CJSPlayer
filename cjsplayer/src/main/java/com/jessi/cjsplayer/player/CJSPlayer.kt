package com.jessi.cjsplayer.player

import android.view.SurfaceView
import com.jessi.cjsplayer.base.IPlayerManager
import com.jessi.cjsplayer.base.OnErrorListener
import com.jessi.cjsplayer.base.OnPreparedListener
import com.jessi.cjsplayer.base.OnProgressListener

class CJSPlayer : IPlayerManager {

    private var playerManager : IPlayerManager? = null

    init {
        playerManager = PlayerFactory.getPlayerManager()
    }


    override fun setSurfaceView(surfaceView: SurfaceView){
        playerManager?.setSurfaceView(surfaceView)
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

    override fun getDuration(): Int {
        return playerManager?.getDuration() ?: 0
    }

    override fun seek(secs: Int) {
        playerManager?.seek(secs)
    }

    override fun setOnPreparedListener(onPreparedListener: OnPreparedListener) {
        playerManager?.setOnPreparedListener(onPreparedListener)
    }

    override fun setOnErrorListener(onErrorListener: OnErrorListener) {
        playerManager?.setOnErrorListener(onErrorListener)
    }

    override fun setOnProgressListen(onProgressListener: OnProgressListener) {
        playerManager?.setOnProgressListen(onProgressListener)
    }


}