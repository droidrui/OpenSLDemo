//
// Created by lingrui on 2017/3/24.
//

#ifndef OPENSLDEMO_OPENSL_H
#define OPENSLDEMO_OPENSL_H

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <pthread.h>
#include <stdlib.h>

typedef struct ThreadLock_ {

    pthread_mutex_t m;
    pthread_cond_t c;
    unsigned char s;

} ThreadLock;

typedef struct OpenSLEngine_ {

    SLObjectItf engineObj;
    SLEngineItf engineItf;

    SLObjectItf outputMixObj;

    SLObjectItf recorderObj;
    SLRecordItf recorderItf;
    SLAndroidSimpleBufferQueueItf recorderBufferQueueItf;

    SLObjectItf playerObj;
    SLPlayItf playerItf;
    SLAndroidSimpleBufferQueueItf playerBufferQueueItf;

    int recordBufferSize;
    int recordChannel;
    int recordSampleRate;
    short *recordBuffer[2];
    int currRecordBufferIndex;
    int currRecordReadIndex;
    ThreadLock *recordLock;

    int playBufferSize;
    int playChannel;
    int playSampleRate;
    short *playBuffer[2];
    int currPlayBufferIndex;
    int currPlayWriteIndex;
    ThreadLock *playLock;

} OpenSLEngine;

OpenSLEngine *createEngine(void);

void createRecorder(OpenSLEngine *engine, int sampleRate, int numChannel, int bufferSize);

void createPlayer(OpenSLEngine *engine, int sampleRate, int numChannel, int bufferSize);

void destroyEngine(OpenSLEngine *engine);

void readFromRecorder(OpenSLEngine *engine, short *buffer, int size);

void writeToPlayer(OpenSLEngine *engine, short *buffer, int size);

#endif //OPENSLDEMO_OPENSL_H
