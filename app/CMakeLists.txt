cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNO_CRYPTO")
include_directories(src/main/cpp/include)
include_directories(src/main/cpp/librtmp)
include_directories(src/main/cpp/push)
include_directories(src/main/cpp/soundtouch/include)
include_directories(src/main/cpp/soundtouch/SoundTouch)

add_library( # Sets the name of the library.
        native-lib
        SHARED

        src/main/cpp/native-lib.cpp
        src/main/cpp/CyCallJava.cpp
        src/main/cpp/CyAudio.cpp
        src/main/cpp/CyFFmpeg.cpp
        src/main/cpp/CyQueue.cpp
        src/main/cpp/CyPlaystatus.cpp
        src/main/cpp/CyPcmBean.cpp
        src/main/cpp/CyBufferQueue.cpp
        src/main/cpp/CyVideo.cpp

        src/main/cpp/push/RtmpPush.cpp
        src/main/cpp/push/CyPushQueue.cpp
        src/main/cpp/push/CyPushCallJava.cpp

        src/main/cpp/RecordBuffer.cpp

        src/main/cpp/soundtouch/SoundTouch/AAFilter.cpp
        src/main/cpp/soundtouch/SoundTouch/FIFOSampleBuffer.cpp
        src/main/cpp/soundtouch/SoundTouch/FIRFilter.cpp
        src/main/cpp/soundtouch/SoundTouch/cpu_detect_x86.cpp
        src/main/cpp/soundtouch/SoundTouch/sse_optimized.cpp
        src/main/cpp/soundtouch/SoundTouch/RateTransposer.cpp
        src/main/cpp/soundtouch/SoundTouch/SoundTouch.cpp
        src/main/cpp/soundtouch/SoundTouch/InterpolateCubic.cpp
        src/main/cpp/soundtouch/SoundTouch/InterpolateLinear.cpp
        src/main/cpp/soundtouch/SoundTouch/InterpolateShannon.cpp
        src/main/cpp/soundtouch/SoundTouch/TDStretch.cpp
        src/main/cpp/soundtouch/SoundTouch/PeakFinder.cpp

        src/main/cpp/librtmp/amf.c
        src/main/cpp/librtmp/hashswf.c
        src/main/cpp/librtmp/log.c
        src/main/cpp/librtmp/parseurl.c
        src/main/cpp/librtmp/rtmp.c
        )

# Import ffmpeg part
add_library(avcodec
        SHARED
        IMPORTED)
set_target_properties(avcodec
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libavcodec.so)

add_library(avdevice
        SHARED
        IMPORTED)
set_target_properties(avdevice
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libavdevice.so)

add_library(avfilter
        SHARED
        IMPORTED)
set_target_properties(avfilter
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libavfilter.so)

add_library(avformat
        SHARED
        IMPORTED)
set_target_properties(avformat
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libavformat.so)

add_library(avutil
        SHARED
        IMPORTED)
set_target_properties(avutil
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libavutil.so)

add_library( postproc SHARED IMPORTED)
set_target_properties( postproc
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libpostproc.so)

add_library(swresample
        SHARED
        IMPORTED)
set_target_properties(swresample
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libswresample.so)

add_library(swscale
        SHARED
        IMPORTED)
set_target_properties(swscale
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libswscale.so)

target_link_libraries( # Specifies the target library.
        native-lib
        avcodec
        avdevice
        avfilter
        avformat
        avutil
        postproc
        swresample
        swscale

        OpenSLES
        android

        log)
message(STATUS "test>>>>>>>>>>>")