package com.chen.cyplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.os.Environment;
import android.view.View;

import com.chen.cyplayer.encodec.CyMediaEncodec;
import com.chen.cyplayer.imgvideo.CyImgVideoView;
import com.ywl5320.libmusic.WlMusic;
import com.ywl5320.listener.OnPreparedListener;
import com.ywl5320.listener.OnShowPcmDataListener;

public class Main3Activity extends AppCompatActivity {
    private CyImgVideoView cyImgVideoView;
    private CyMediaEncodec cyMediaEncodec;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main3);

        cyImgVideoView = findViewById(R.id.imgvideoview);
        cyImgVideoView.setCurrentImg(R.drawable.girl);




//                cyMediaEncodec = new CyMediaEncodec(Main3Activity.this, cyImgVideoView.getFbotextureid());
//                cyMediaEncodec.initEncodec(cyImgVideoView.getEglContext(),
//                        Environment.getExternalStorageDirectory().getAbsolutePath() + "/image_video.mp4",
//                        720, 500, samplerate, channels);
//                cyMediaEncodec.startRecord();
//                startImgs();

    }

    public void start(View view) {

    }

    private void startImgs() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                for (int i = 1; i <= 10; i++) {
                    int imgsrc = getResources().getIdentifier("img_" + i, "drawable", "com.chen.cyplayer");
                    cyImgVideoView.setCurrentImg(imgsrc);
                    try {
                        Thread.sleep(1000);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }

                if (cyMediaEncodec != null) {
                    cyMediaEncodec.stopRecord();
                    cyMediaEncodec = null;
                }
            }
        }).start();
    }
}
