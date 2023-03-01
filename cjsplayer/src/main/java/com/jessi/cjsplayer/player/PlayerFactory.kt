package com.jessi.cjsplayer.player

import com.jessi.cjsplayer.base.IPlayerManager
import com.jessi.cjsplayer.manager.CJSPlayerManager

/**
 * 设置播放内核
 */
object PlayerFactory {

    private var sPlayerManager: Class<out IPlayerManager>? = null


    fun setPlayerManager(playerManager : Class<out IPlayerManager>){
        sPlayerManager = playerManager
    }

    fun getPlayerManager(): IPlayerManager{
        return sPlayerManager?.newInstance() ?: CJSPlayerManager::class.java.newInstance()
    }
}