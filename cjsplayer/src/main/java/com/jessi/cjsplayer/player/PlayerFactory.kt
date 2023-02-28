package com.jessi.cjsplayer.player

import com.jessi.cjsplayer.base.IPlayerManager
import com.jessi.cjsplayer.manager.CJSPlayerManager


object PlayerFactory {

    private var sPlayerManager: Class<out IPlayerManager>? = null


    fun setPlayerManager(playerManager : Class<out IPlayerManager>){
        sPlayerManager = playerManager
    }

    fun getPlayerManager(): IPlayerManager{
        return sPlayerManager?.newInstance() ?: CJSPlayerManager::class.java.newInstance()
    }
}