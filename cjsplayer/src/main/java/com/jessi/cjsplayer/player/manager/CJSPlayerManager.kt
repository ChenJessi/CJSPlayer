package com.jessi.cjsplayer.player.manager

import android.util.Log
import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView
import com.jessi.cjsplayer.player.base.CJSBasePlayerManager
import com.jessi.cjsplayer.player.base.IPlayerManager
import com.jessi.cjsplayer.player.base.OnErrorListener
import com.jessi.cjsplayer.player.base.OnPreparedListener
import kotlin.math.log

private const val TAG = "CJSPlayerManager"
// 打开媒体url 失败
private const val CODE_OPEN_URL_FAIL  = 101
// 无法找到流媒体
private const val CODE_FIND_STREAMS_FAIL  = 102
// 找不到解码器
private const val CODE_FIND_DECODER_FAIL  = 103
// 无法创建解码器上下文
private const val CODE_ALLOC_NEW_CODEC_CONTEXT_FAIL = 104
// 填充解码器上下文参数失败
private const val CODE_FILL_CODEC_CONTEXT_FAIL  = 105
// 打开解码器失败
private const val CODE_OPEN_CODEC_FAIL  = 106
// 没有音视频媒体信息
private const val CODE_NOT_MEDIA  = 107

internal class CJSPlayerManager : IPlayerManager, CJSBasePlayerManager() , SurfaceHolder.Callback{

    init {
        System.loadLibrary("cjsplayer-native-lib")
    }

    private var surfaceHolder : SurfaceHolder? = null

    override fun setDataSource(source : String) {
        this.dataSource = source
    }

    override fun prepare() {
        prepareNative(dataSource)
    }

    override fun start() {
        startNative()
    }

    override fun stop() {

    }

    override fun release() {

    }

    override fun setOnPreparedListener(onPreparedListener : OnPreparedListener) {
        this.onPreparedListener = onPreparedListener
    }

    override fun setOnErrorListener(onErrorListener: OnErrorListener) {
        this.onErrorListener = onErrorListener
    }


    private fun onPrepared(){
        onPreparedListener?.invoke()
    }
    private fun onError(code :Int){
        val errorMsg = when(code){
            CODE_OPEN_URL_FAIL -> "CODE_OPEN_URL_FAIL"
            CODE_FIND_STREAMS_FAIL -> "CODE_FIND_STREAMS_FAIL"
            CODE_FIND_DECODER_FAIL -> "CODE_FIND_DECODER_FAIL"
            CODE_ALLOC_NEW_CODEC_CONTEXT_FAIL -> "CODE_ALLOC_NEW_CODEC_CONTEXT_FAIL"
            CODE_FILL_CODEC_CONTEXT_FAIL -> "CODE_FILL_CODEC_CONTEXT_FAIL"
            CODE_OPEN_CODEC_FAIL -> "CODE_OPEN_CODEC_FAIL"
            CODE_NOT_MEDIA -> "CODE_NOT_MEDIA"
            else -> "UNKNOWN_ERROR"
        }
        onErrorListener?.invoke(errorMsg)
    }



    fun setSurfaceView(surfaceView: SurfaceView){
        // 重新设置之后清除上一次的
        surfaceHolder?.removeCallback(this)

        surfaceHolder = surfaceView.holder
        surfaceHolder?.addCallback(this)
    }
    override fun surfaceCreated(holder: SurfaceHolder) {

    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        setSurfaceNative(holder.surface)
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {

    }



    private external fun prepareNative(source :String)
    private external fun startNative()
    private external fun stopNative()
    private external fun releaseNative()

    private external fun setSurfaceNative(surface: Surface);
}