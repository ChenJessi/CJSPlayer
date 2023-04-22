cmake_minimum_required(VERSION 3.6.0)

file(GLOB faac_src *.c libfaac/*.c)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNO_CRYPTO")

add_library(faac STATIC ${faac_src})