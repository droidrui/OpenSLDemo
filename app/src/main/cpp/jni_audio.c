//
// Created by lingrui on 2017/3/24.
//
#include <jni.h>
#include <stdio.h>
#include "opensl.h"

static OpenSLEngine *engine;
static FILE *recordFile;
static FILE *playFile;
static int recordBufferSize;
static int playBufferSize;
static volatile int g_loop_exit = 0;

JNIEXPORT void JNICALL Java_com_droidrui_opensldemo_jni_AudioJNI_initRecord
        (JNIEnv *env, jobject thiz, jint sampleRate, jint numChannel, jint framesPerBuffer,
         jstring filePath) {
    recordBufferSize = framesPerBuffer * numChannel * 4;
    engine = createEngine();
    createRecorder(engine, sampleRate, numChannel, framesPerBuffer);
    const char *path = (*env)->GetStringUTFChars(env, filePath, NULL);
    recordFile = fopen(path, "wb");
    (*env)->ReleaseStringUTFChars(env, filePath, path);
}

void *record(void *context) {
    short buffer[recordBufferSize];
    g_loop_exit = 0;
    while (!g_loop_exit) {
        readFromRecorder(engine, buffer, recordBufferSize);
        fwrite(buffer, sizeof(short), recordBufferSize, recordFile);
    }
    destroyEngine(engine);
    fclose(recordFile);
}

JNIEXPORT void JNICALL Java_com_droidrui_opensldemo_jni_AudioJNI_startRecord
        (JNIEnv *env, jobject thiz) {
    pthread_t threadInfo_;
    pthread_attr_t threadAttr_;

    pthread_attr_init(&threadAttr_);
    pthread_attr_setdetachstate(&threadAttr_, PTHREAD_CREATE_DETACHED);

    pthread_create(&threadInfo_, &threadAttr_, record, NULL);
}


JNIEXPORT void JNICALL Java_com_droidrui_opensldemo_jni_AudioJNI_pauseRecord
        (JNIEnv *env, jobject thiz) {

}


JNIEXPORT void JNICALL Java_com_droidrui_opensldemo_jni_AudioJNI_stopRecord
        (JNIEnv *env, jobject thiz) {
    g_loop_exit = 1;
}

JNIEXPORT void JNICALL Java_com_droidrui_opensldemo_jni_AudioJNI_initPlay
        (JNIEnv *env, jobject thiz, jint sampleRate, jint numChannel, jint framesPerBuffer,
         jstring filePath) {
    playBufferSize = framesPerBuffer * numChannel * 4;
    engine = createEngine();
    createPlayer(engine, sampleRate, numChannel, framesPerBuffer);
    const char *path = (*env)->GetStringUTFChars(env, filePath, NULL);
    playFile = fopen(path, "rb");
    (*env)->ReleaseStringUTFChars(env, filePath, path);
}

void *play(void *context) {
    short buffer[playBufferSize];
    g_loop_exit = 0;
    while (!g_loop_exit && !feof(playFile)) {
        fread(buffer, sizeof(short), playBufferSize, playFile);
        writeToPlayer(engine, buffer, playBufferSize);
    }
    destroyEngine(engine);
    fclose(playFile);
}

JNIEXPORT void JNICALL Java_com_droidrui_opensldemo_jni_AudioJNI_startPlay
        (JNIEnv *env, jobject thiz) {
    pthread_t threadInfo_;
    pthread_attr_t threadAttr_;

    pthread_attr_init(&threadAttr_);
    pthread_attr_setdetachstate(&threadAttr_, PTHREAD_CREATE_DETACHED);

    pthread_create(&threadInfo_, &threadAttr_, play, NULL);
}


JNIEXPORT void JNICALL Java_com_droidrui_opensldemo_jni_AudioJNI_pausePlay
        (JNIEnv *env, jobject thiz) {

}


JNIEXPORT void JNICALL Java_com_droidrui_opensldemo_jni_AudioJNI_stopPlay
        (JNIEnv *env, jobject thiz) {
    g_loop_exit = 1;
}
