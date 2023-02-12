package com.jessi.cjsplayer.player.manager

internal class CJSPlayerManager : IPlayerManager{

    init {
        System.loadLibrary("cjsplayer-native-lib")
    }

    override fun setUrl(url: String) {
        TODO("Not yet implemented")
    }









}