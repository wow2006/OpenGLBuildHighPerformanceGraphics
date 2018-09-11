package com.ahussein.simple;

import android.content.Context;
import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class GL3JNIView extends GLSurfaceView {
    public GL3JNIView(Context context) {
        super(context);

        setEGLConfigChooser(8, 8, 8, 0, 16, 0);
        setEGLContextClientVersion(3);
        setRenderer(new Renderer());
    }

    public static class Renderer implements GLSurfaceView.Renderer {
        @Override
        public void onSurfaceCreated(GL10 gl, EGLConfig config) {}

        @Override
        public void onSurfaceChanged(GL10 gl, int width, int height) {
            GL3JNILib.init(width, height);
        }

        @Override
        public void onDrawFrame(GL10 gl) {
            GL3JNILib.step();
        }
    }
}
