package com.chen.cjsplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;

import com.chen.cjsplayer.log.MyLog;
import com.chen.cjsplayer.yuv.CyYuvView;

import java.io.File;
import java.io.FileInputStream;

public class Main4Activity extends AppCompatActivity {
    private CyYuvView cyYuvView;
    private FileInputStream fis;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main4);
        cyYuvView = findViewById(R.id.yuvview);
    }

    public void start(View view) {
        new Thread(new Runnable() {
            @Override
            public void run() {

                try {
                    int w = 640;
                    int h = 360;
                    fis = new FileInputStream(new File("/mnt/sdcard/tencent/QQfile_recv/sintel_640_360.yuv"));
                    byte[] y = new byte[w * h];
                    byte[] u = new byte[w * h / 4];
                    byte[] v = new byte[w * h / 4];

                    while (true) {
                        int ry = fis.read(y);
                        int ru = fis.read(u);
                        int rv = fis.read(v);
                        if (ry > 0 && ru > 0 && rv > 0) {
                            MyLog.d("开始" + w);
                            cyYuvView.setFrameData(w, h, y, u, v);
                            Thread.sleep(40);
                        } else {
                            MyLog.d("完成");
                            break;
                        }
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }).start();
    }
}
