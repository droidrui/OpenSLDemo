
cmake_minimum_required(VERSION 3.4.1)

add_library(audio SHARED
            jni_audio.c
            opensl.c
           )

target_link_libraries(audio
                      android
                      log
                      OpenSLES
                     )