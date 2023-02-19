package com.jessi.cjsplayer.player.base

typealias OnPreparedListener = (()->Unit)?
typealias OnErrorListener = ((String)->Unit)?
abstract class CJSBasePlayerManager {


    // 数据源
    @JvmField
    var dataSource : String = ""
    // 准备回调
    @JvmField
    var onPreparedListener : OnPreparedListener = null
    // 失败回调
    @JvmField
    var onErrorListener : OnErrorListener = null

}