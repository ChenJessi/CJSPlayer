package com.chen.cyplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.opengl.GLES20;
import android.os.Bundle;
import android.print.PrinterId;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import com.chen.cyplayer.opengl.EglHelper;
import com.chen.test.CyGLSurfaceView;

public class TestActivity extends AppCompatActivity {
    private CyGLSurfaceView surfaceView;
    private EglHelper eglHelper;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_test);
        surfaceView = findViewById(R.id.surfaceView);

    }
}
