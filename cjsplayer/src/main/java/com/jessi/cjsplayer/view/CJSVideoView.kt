package com.jessi.cjsplayer.view

import android.content.Context
import android.util.AttributeSet
import android.view.SurfaceView
import com.jessi.cjsplayer.CJSPlayer
import com.jessi.cjsplayer.base.IPlayerManager
import com.jessi.cjsplayer.base.OnErrorListener
import com.jessi.cjsplayer.base.OnPreparedListener


class CJSVideoView @JvmOverloads constructor(
    context: Context, attrs: AttributeSet? = null
) : SurfaceView(context, attrs), IPlayerManager {

    private var cjsPlayer : CJSPlayer? = null
    init {
        cjsPlayer = CJSPlayer()
        setSurfaceView(this)
    }

    override fun setDataSource(source: String) {
        cjsPlayer?.setDataSource(source)
    }

    override fun setSurfaceView(surfaceView: SurfaceView) {
        cjsPlayer?.setSurfaceView(surfaceView)
    }

    override fun prepare() {
        cjsPlayer?.prepare()
    }

    override fun start() {
        cjsPlayer?.start()
    }

    override fun stop() {
        cjsPlayer?.start()
    }

    override fun release() {
        cjsPlayer?.release()
    }

    override fun setOnPreparedListener(onPreparedListener: OnPreparedListener) {
        cjsPlayer?.setOnPreparedListener(onPreparedListener)
    }

    override fun setOnErrorListener(onErrorListener: OnErrorListener) {
        cjsPlayer?.setOnErrorListener(onErrorListener)
    }
}