cmake_minimum_required(VERSION 3.6.0)


# 声明 ffmpeg 路径
set(FFMPEG ${CMAKE_SOURCE_DIR}/ffmpeg)
# 导入头文件
include_directories(${FFMPEG}/include)

#指定 ffmpeg 库 路径
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${FFMPEG}/libs/${CMAKE_ANDROID_ARCH_ABI}")


file(GLOB resource *.cpp *.h player/*.cpp player/*.h)

add_library(
        cjsplayer-native-lib
        SHARED
        ${resource}
)


target_link_libraries(
        cjsplayer-native-lib

        #-Wl, --start-group
        avformat avcodec avformat avfilter avutil swresample swscale
        #-Wl, --end-group

        android
        OpenSLES
        z
        log
)
message("message >>>>>>>>>>>>")