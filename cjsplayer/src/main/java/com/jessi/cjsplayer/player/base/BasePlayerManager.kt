package com.jessi.cjsplayer.player.base

typealias OnPreparedListener = (()->Unit)?
abstract class CJSBasePlayerManager {


    // 数据源
    @JvmField
    var dataSource : String = ""
    // 准备回调
    @JvmField
    var onPreparedListener : OnPreparedListener = null
}