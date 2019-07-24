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
        cyPushVideo.initLivePush("rtmp://send3a.douyu.com/live/6441662rDbA37ogG?wsSecret=4b1f5f759806677a3c034479dcb6eb6e&wsTime=5d383fb9&wsSeek=off&wm=0&tw=0&roirecognition=0");
    }
}
