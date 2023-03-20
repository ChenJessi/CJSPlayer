cmake_minimum_required(VERSION 3.6.0)


# 声明 ffmpeg 路径
set(FFMPEG ${CMAKE_SOURCE_DIR}/ffmpeg)
# 导入头文件
include_directories(${FFMPEG}/include)
#指定 ffmpeg 库 路径
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${FFMPEG}/libs/${CMAKE_ANDROID_ARCH_ABI}")


# 添加 openssl
set(OPENSSL ${CMAKE_SOURCE_DIR}/openssl)
include_directories(${OPENSSL}/include/openssl)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${OPENSSL}/libs/${CMAKE_ANDROID_ARCH_ABI}")

# 添加 rtmp
set(RTMP ${CMAKE_SOURCE_DIR}/rtmp)
include_directories(${RTMP}/include)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${RTMP}/libs/${CMAKE_ANDROID_ARCH_ABI}")

file(GLOB resource  *.cpp  */*.cpp )

add_library(
        cjsplayer-native-lib
        SHARED
        ${resource}
)


target_link_libraries(
        cjsplayer-native-lib

        #-Wl, --start-group
        avformat
        avcodec
        avfilter
        avutil
        swresample
        swscale
        #-Wl, --end-group

        ssl
        crypto

        rtmp

        OpenSLES
        z
        android
        log
)

message("message >>>>>>>>>>>> ${FFMPEG}  ${CMAKE_ANDROID_ARCH_ABI}  ${OPENSSL} ")