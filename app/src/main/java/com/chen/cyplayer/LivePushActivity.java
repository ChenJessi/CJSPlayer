package com.chen.cyplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;

import com.chen.cyplayer.push.CyPushVideo;

public class LivePushActivity extends AppCompatActivity {

    private CyPushVideo cyPushVideo;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_live_push);
        cyPushVideo = new CyPushVideo();
    }

    public void push(View view) {
        cyPushVideo.initLivePush("rtmp://219.232.160.120/livestream/3c7b0e7ffa2bb75c01aa7635cb7cc12f");
    }
}
