package com.ahussein.simple;

public class GL3JNILib {
    static {
        System.loadLibrary("gl3jni");
    }

    public static native void init(int width, int height);

    public static native void step();
}
