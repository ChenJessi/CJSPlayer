package com.chen.cyplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.opengl.GLES20;
import android.os.Bundle;
import android.print.PrinterId;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import com.chen.cyplayer.opengl.EglHelper;

public class TestActivity extends AppCompatActivity {
    private SurfaceView surfaceView;
    private EglHelper eglHelper;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_test);
        surfaceView = findViewById(R.id.surfaceView);
        surfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {

            }

            @Override
            public void surfaceChanged(final SurfaceHolder holder, int format, final int width, final int height) {
                new Thread(){
                    @Override
                    public void run() {
                        super.run();
                        eglHelper = new EglHelper();
                        eglHelper.initEgl(holder.getSurface(), null);
                        while (true){
                            GLES20.glViewport(0,0, width, height);
                            GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
                            GLES20.glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
                            eglHelper.swapBuffers();
                        }
                    }
                }.start();
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {

            }
        });
    }
}
