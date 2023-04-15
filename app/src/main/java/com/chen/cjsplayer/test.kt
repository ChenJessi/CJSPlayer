package com.chen.cjsplayer

import kotlinx.coroutines.Job
import kotlin.coroutines.AbstractCoroutineContextElement
import kotlin.coroutines.Continuation
import kotlin.coroutines.ContinuationInterceptor
import kotlin.coroutines.CoroutineContext


class CoroutineName() : AbstractCoroutineContextElement(Job.Key) {
    companion object Key : CoroutineContext.Key<CoroutineName>
}

class LogInterceptor(override val key: CoroutineContext.Key<*>) : ContinuationInterceptor{
    override fun <T> interceptContinuation(continuation: Continuation<T>): Continuation<T> {
        TODO("Not yet implemented")

    }

}