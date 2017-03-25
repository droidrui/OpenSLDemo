//
// Created by lingrui on 2017/3/24.
//

#include "opensl.h"

static ThreadLock *createThreadLock(void);

static void waitThreadLock(ThreadLock *lock);

static void notifyThreadLock(ThreadLock *lock);

static void destroyThreadLock(ThreadLock *lock);

static void recordCallback(SLAndroidSimpleBufferQueueItf bq, void *context);

static void playCallback(SLAndroidSimpleBufferQueueItf bq, void *context);


OpenSLEngine *createEngine(void) {
    OpenSLEngine *engine = (OpenSLEngine *) calloc(1, sizeof(OpenSLEngine));
    slCreateEngine(&(engine->engineObj), 0, NULL, 0, NULL, NULL);
    (*engine->engineObj)->Realize(engine->engineObj, SL_BOOLEAN_FALSE);
    (*engine->engineObj)->GetInterface(engine->engineObj, SL_IID_ENGINE, &(engine->engineItf));
    return engine;
}

void createRecorder(OpenSLEngine *engine, int sampleRate, int numChannel, int bufferSize) {
    engine->recordSampleRate = sampleRate;
    engine->recordChannel = numChannel;
    engine->recordBufferSize = bufferSize * numChannel;

    engine->recordLock = createThreadLock();
    engine->recordBuffer[0] = (short *) calloc(engine->recordBufferSize, sizeof(short));
    engine->recordBuffer[1] = (short *) calloc(engine->recordBufferSize, sizeof(short));

    engine->currRecordBufferIndex = 0;
    engine->currRecordReadIndex = engine->recordBufferSize;

    notifyThreadLock(engine->recordLock);

    SLuint32 sr = sampleRate;
    switch (sr) {
        case 8000:
            sr = SL_SAMPLINGRATE_8;
            break;
        case 11025:
            sr = SL_SAMPLINGRATE_11_025;
            break;
        case 16000:
            sr = SL_SAMPLINGRATE_16;
            break;
        case 22050:
            sr = SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            sr = SL_SAMPLINGRATE_24;
            break;
        case 32000:
            sr = SL_SAMPLINGRATE_32;
            break;
        case 44100:
            sr = SL_SAMPLINGRATE_44_1;
            break;
        case 48000:
            sr = SL_SAMPLINGRATE_48;
            break;
        case 64000:
            sr = SL_SAMPLINGRATE_64;
            break;
        case 88200:
            sr = SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            sr = SL_SAMPLINGRATE_96;
            break;
        case 192000:
            sr = SL_SAMPLINGRATE_192;
            break;
        default:
            sr = SL_SAMPLINGRATE_44_1;
    }
    SLDataLocator_IODevice srcLocat = {SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT,
                                       SL_DEFAULTDEVICEID_AUDIOINPUT, NULL};
    SLDataSource dataSrc = {&srcLocat, NULL};

    SLDataLocator_AndroidSimpleBufferQueue snkLocat = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLuint32 channelMask = SL_SPEAKER_FRONT_CENTER;
    if (numChannel > 1) {
        channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    }
    SLDataFormat_PCM snkFmt = {
            SL_DATAFORMAT_PCM,
            numChannel,
            sr,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            channelMask,
            SL_BYTEORDER_LITTLEENDIAN
    };
    SLDataSink dataSnk = {&snkLocat, &snkFmt};

    SLInterfaceID ids[] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    SLboolean reqs[] = {SL_BOOLEAN_TRUE};
    (*engine->engineItf)->CreateAudioRecorder(engine->engineItf, &(engine->recorderObj), &dataSrc,
                                              &dataSnk, 1, ids, reqs);
    (*engine->recorderObj)->Realize(engine->recorderObj, SL_BOOLEAN_FALSE);
    (*engine->recorderObj)->GetInterface(engine->recorderObj, SL_IID_RECORD,
                                         &(engine->recorderItf));
    (*engine->recorderObj)->GetInterface(engine->recorderObj, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                         &(engine->recorderBufferQueueItf));
    (*engine->recorderBufferQueueItf)->RegisterCallback(engine->recorderBufferQueueItf,
                                                        recordCallback, engine);
    (*engine->recorderItf)->SetRecordState(engine->recorderItf, SL_RECORDSTATE_RECORDING);
}

void createPlayer(OpenSLEngine *engine, int sampleRate, int numChannel, int bufferSize) {
    engine->playSampleRate = sampleRate;
    engine->playChannel = numChannel;
    engine->playBufferSize = bufferSize * numChannel;

    engine->playLock = createThreadLock();
    engine->playBuffer[0] = (short *) calloc(engine->playBufferSize, sizeof(short));
    engine->playBuffer[1] = (short *) calloc(engine->playBufferSize, sizeof(short));

    engine->currPlayBufferIndex = 0;
    engine->currPlayWriteIndex = 0;

    notifyThreadLock(engine->playLock);

    SLuint32 sr = sampleRate;
    switch (sr) {
        case 8000:
            sr = SL_SAMPLINGRATE_8;
            break;
        case 11025:
            sr = SL_SAMPLINGRATE_11_025;
            break;
        case 16000:
            sr = SL_SAMPLINGRATE_16;
            break;
        case 22050:
            sr = SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            sr = SL_SAMPLINGRATE_24;
            break;
        case 32000:
            sr = SL_SAMPLINGRATE_32;
            break;
        case 44100:
            sr = SL_SAMPLINGRATE_44_1;
            break;
        case 48000:
            sr = SL_SAMPLINGRATE_48;
            break;
        case 64000:
            sr = SL_SAMPLINGRATE_64;
            break;
        case 88200:
            sr = SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            sr = SL_SAMPLINGRATE_96;
            break;
        case 192000:
            sr = SL_SAMPLINGRATE_192;
            break;
        default:
            sr = SL_SAMPLINGRATE_44_1;
    }
    SLDataLocator_AndroidSimpleBufferQueue srcLocat = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLuint32 channelMask = SL_SPEAKER_FRONT_CENTER;
    if (numChannel > 1) {
        channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    }
    SLDataFormat_PCM srcFmt = {
            SL_DATAFORMAT_PCM,
            numChannel,
            sr,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            channelMask,
            SL_BYTEORDER_LITTLEENDIAN
    };
    SLDataSource dataSrc = {&srcLocat, &srcFmt};

    (*engine->engineItf)->CreateOutputMix(engine->engineItf, &(engine->outputMixObj), 0, NULL,
                                          NULL);
    (*engine->outputMixObj)->Realize(engine->outputMixObj, SL_BOOLEAN_FALSE);
    SLDataLocator_OutputMix snkLocat = {SL_DATALOCATOR_OUTPUTMIX, engine->outputMixObj};
    SLDataSink dataSnk = {&snkLocat, NULL};

    SLInterfaceID ids[] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    SLboolean reqs[] = {SL_BOOLEAN_TRUE};
    (*engine->engineItf)->CreateAudioPlayer(engine->engineItf, &(engine->playerObj), &dataSrc,
                                            &dataSnk, 1, ids, reqs);
    (*engine->playerObj)->Realize(engine->playerObj, SL_BOOLEAN_FALSE);
    (*engine->playerObj)->GetInterface(engine->playerObj, SL_IID_PLAY, &(engine->playerItf));
    (*engine->playerObj)->GetInterface(engine->playerObj, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                       &(engine->playerBufferQueueItf));
    (*engine->playerBufferQueueItf)->RegisterCallback(engine->playerBufferQueueItf, playCallback,
                                                      engine);
    (*engine->playerItf)->SetPlayState(engine->playerItf, SL_PLAYSTATE_PLAYING);
}

void destroyEngine(OpenSLEngine *engine) {
    if (engine->playerObj != NULL) {
        (*engine->playerObj)->Destroy(engine->playerObj);
        engine->playerObj = NULL;
        engine->playerItf = NULL;
        engine->playerBufferQueueItf = NULL;
    }

    if (engine->recorderObj != NULL) {
        (*engine->recorderObj)->Destroy(engine->recorderObj);
        engine->recorderObj = NULL;
        engine->recorderItf = NULL;
        engine->recorderBufferQueueItf = NULL;
    }

    if (engine->outputMixObj != NULL) {
        (*engine->outputMixObj)->Destroy(engine->outputMixObj);
        engine->outputMixObj = NULL;
    }

    if (engine->engineObj != NULL) {
        (*engine->engineObj)->Destroy(engine->engineObj);
        engine->engineObj = NULL;
        engine->engineItf = NULL;
    }

    if (engine->recordLock != NULL) {
        notifyThreadLock(engine->recordLock);
        destroyThreadLock(engine->recordLock);
        engine->recordLock = NULL;
    }

    if (engine->playLock != NULL) {
        notifyThreadLock(engine->playLock);
        destroyThreadLock(engine->playLock);
        engine->playLock = NULL;
    }

    if (engine->recordBuffer[0] != NULL) {
        free(engine->recordBuffer[0]);
        engine->recordBuffer[0] = NULL;
    }

    if (engine->recordBuffer[1] != NULL) {
        free(engine->recordBuffer[1]);
        engine->recordBuffer[1] = NULL;
    }

    if (engine->playBuffer[0] != NULL) {
        free(engine->playBuffer[0]);
        engine->playBuffer[0] = NULL;
    }

    if (engine->playBuffer[1] != NULL) {
        free(engine->playBuffer[1]);
        engine->playBuffer[1] = NULL;
    }

    free(engine);
}

void recordCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    OpenSLEngine *engine = (OpenSLEngine *) context;
    notifyThreadLock(engine->recordLock);
}

void readFromRecorder(OpenSLEngine *engine, short *buffer, int size) {
    int index = engine->currRecordReadIndex;
    short *recordBuffer = engine->recordBuffer[engine->currRecordBufferIndex];
    for (int i = 0; i < size; i++) {
        if (index >= engine->recordBufferSize) {
            waitThreadLock(engine->recordLock);
            (*engine->recorderBufferQueueItf)->Enqueue(engine->recorderBufferQueueItf, recordBuffer,
                                                       engine->recordBufferSize * sizeof(short));
            engine->currRecordBufferIndex = (engine->currRecordBufferIndex ? 0 : 1);
            index = 0;
            recordBuffer = engine->recordBuffer[engine->currRecordBufferIndex];
        }
        buffer[i] = recordBuffer[index++];
    }
    engine->currRecordReadIndex = index;
}


void playCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    OpenSLEngine *engine = (OpenSLEngine *) context;
    notifyThreadLock(engine->playLock);
}

void writeToPlayer(OpenSLEngine *engine, short *buffer, int size) {
    int index = engine->currPlayWriteIndex;
    short *playBuffer = engine->playBuffer[engine->currPlayBufferIndex];
    for (int i = 0; i < size; i++) {
        playBuffer[index++] = buffer[i];
        if (index >= engine->playBufferSize) {
            waitThreadLock(engine->playLock);
            (*engine->playerBufferQueueItf)->Enqueue(engine->playerBufferQueueItf,
                                                     playBuffer,
                                                     engine->playBufferSize * sizeof(short));
            engine->currPlayBufferIndex = (engine->currPlayBufferIndex ? 0 : 1);
            index = 0;
            playBuffer = engine->playBuffer[engine->currPlayBufferIndex];
        }
    }
    engine->currPlayWriteIndex = index;
}

ThreadLock *createThreadLock(void) {
    ThreadLock *lock = (ThreadLock *) calloc(1, sizeof(ThreadLock));
    pthread_mutex_init(&(lock->m), NULL);
    pthread_cond_init(&(lock->c), NULL);
    lock->s = 1;
    return lock;
}

void waitThreadLock(ThreadLock *lock) {
    pthread_mutex_lock(&(lock->m));
    while (!lock->s) {
        pthread_cond_wait(&(lock->c), &(lock->m));
    }
    lock->s = 0;
    pthread_mutex_unlock(&(lock->m));
}

void notifyThreadLock(ThreadLock *lock) {
    pthread_mutex_lock(&(lock->m));
    lock->s = 1;
    pthread_cond_signal(&(lock->c));
    pthread_mutex_unlock(&(lock->m));
}

void destroyThreadLock(ThreadLock *lock) {
    if (lock == NULL) {
        return;
    }
    notifyThreadLock(lock);
    pthread_cond_destroy(&(lock->c));
    pthread_mutex_destroy(&(lock->m));
    free(lock);
}


