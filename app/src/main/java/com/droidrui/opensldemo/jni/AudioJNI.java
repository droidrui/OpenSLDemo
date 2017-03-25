package com.droidrui.opensldemo.jni;

/**
 * Created by lingrui on 2017/3/24.
 */

public class AudioJNI {

    public native void initRecord(int sampleRate, int numChannel, int framesPerBuffer, String filePath);

    public native void startRecord();

    public native void pauseRecord();

    public native void stopRecord();

    public native void initPlay(int sampleRate, int numChannel, int framesPerBuffer, String filePath);

    public native void startPlay();

    public native void pausePlay();

    public native void stopPlay();

}
