package com.jessi.cjsplayer.base


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

    @JvmField
    var onProgressListener : OnProgressListener = null


    @JvmField
    internal var mediaPlayerListener : CJSMediaPlayerListener? = null


}