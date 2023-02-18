package com.jessi.cjsplayer.player.manager

import android.util.Log
import com.jessi.cjsplayer.player.base.CJSBasePlayerManager
import com.jessi.cjsplayer.player.base.IPlayerManager
import com.jessi.cjsplayer.player.base.OnPreparedListener
import kotlin.math.log

private const val TAG = "CJSPlayerManager"
internal class CJSPlayerManager : IPlayerManager, CJSBasePlayerManager() {

    init {
        System.loadLibrary("cjsplayer-native-lib")
    }

    override fun setDataSource(source : String) {
        this.dataSource = source
    }

    override fun prepare() {
        prepareNative(dataSource)
    }

    override fun start() {

    }

    override fun stop() {

    }

    override fun release() {

    }

    override fun setOnPreparedListener(onPreparedListener : OnPreparedListener) {
        this.onPreparedListener = onPreparedListener
    }


    private fun onPrepared(){
        onPreparedListener?.invoke()
    }


    private external fun prepareNative(source :String)
    private external fun startNative()
    private external fun stopNative()
    private external fun releaseNative()

}