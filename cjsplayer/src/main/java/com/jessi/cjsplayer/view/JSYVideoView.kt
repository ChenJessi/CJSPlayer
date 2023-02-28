package com.jessi.cjsplayer.view

import com.jessi.cjsplayer.CJSPlayer
import com.jessi.cjsplayer.base.IPlayerManager
import com.jessi.cjsplayer.base.OnErrorListener
import com.jessi.cjsplayer.base.OnPreparedListener

class JSYVideoView : IPlayerManager {

    private var cjsPlayer : CJSPlayer? = null
    init {
        cjsPlayer = CJSPlayer()
    }

    override fun setDataSource(source: String) {
        TODO("Not yet implemented")
    }

    override fun prepare() {
        TODO("Not yet implemented")
    }

    override fun start() {
        TODO("Not yet implemented")
    }

    override fun stop() {
        TODO("Not yet implemented")
    }

    override fun release() {
        TODO("Not yet implemented")
    }

    override fun setOnPreparedListener(onPreparedListener: OnPreparedListener) {
        TODO("Not yet implemented")
    }

    override fun setOnErrorListener(onErrorListener: OnErrorListener) {
        TODO("Not yet implemented")
    }
}