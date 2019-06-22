package com.chen.cyplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.content.pm.ActivityInfo;
import android.hardware.Camera;
import android.os.Bundle;

import com.chen.cyplayer.camera.CyCameraView;
import com.chen.cyplayer.log.MyLog;

public class Main2Activity extends AppCompatActivity {
    private CyCameraView cyCameraView;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main2);
        cyCameraView = findViewById(R.id.surfaceView);
    }
    @Override
    protected void onDestroy() {
        super.onDestroy();
        cyCameraView.onDestory();
    }
}
