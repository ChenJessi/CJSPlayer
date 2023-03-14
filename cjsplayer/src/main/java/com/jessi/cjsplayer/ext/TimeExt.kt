package com.chen.cjsplayer



fun Int.secondToString() : String{
    val seconds = this % 60
    val secStr = if (seconds <= 9) "0$seconds" else "$seconds"

    val minutes = this / 60
    val minStr = if (minutes <= 9) "0$minutes" else "$minutes"

    val hours = minutes / 60
    val hourStr = if (hours <= 9) "0$hours" else "$hours"

    val timeStr = "$minStr:$secStr"
    return if (hours > 0) "$hourStr:$timeStr" else timeStr
}
