package com.jessi.cjsplayer.player

import com.jessi.cjsplayer.player.base.IPlayerManager
import com.jessi.cjsplayer.player.base.OnPreparedListener
import com.jessi.cjsplayer.player.manager.CJSPlayerManager

class CJSPlayer : IPlayerManager{

    private var playerManager : IPlayerManager? = null

    init {
        playerManager = CJSPlayerManager()
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


}