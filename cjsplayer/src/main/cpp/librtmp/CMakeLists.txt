cmake_minimum_required(VERSION 3.6.0)

file(Glob rtmp_src *.c)

add_library(rtmp STATIC ${rtmp_src})