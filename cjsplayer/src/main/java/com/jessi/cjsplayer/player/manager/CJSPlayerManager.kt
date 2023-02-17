package com.jessi.cjsplayer.player.manager

import com.jessi.cjsplayer.player.base.CJSBasePlayerManager
import com.jessi.cjsplayer.player.base.IPlayerManager
import com.jessi.cjsplayer.player.base.OnPreparedListener

internal class CJSPlayerManager : IPlayerManager, CJSBasePlayerManager() {

    init {


        System.loadLibrary("cjsplayer-native-lib")
        System.loadLibrary("avformat");
        System.loadLibrary("avcodec");
        System.loadLibrary("avdevice");
        System.loadLibrary("avfilter");
        System.loadLibrary("avutil");
        System.loadLibrary("postproc");
        System.loadLibrary("swresample");
        System.loadLibrary("swscale");
    }

    override fun setDataSource(source : String) {
        this.dataSource = source
    }

    override fun prepare() {
        prepareNative(dataSource)
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