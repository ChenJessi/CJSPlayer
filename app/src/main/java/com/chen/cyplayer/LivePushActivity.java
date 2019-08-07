package com.chen.cyplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;

import com.chen.cyplayer.camera.CyCameraView;
import com.chen.cyplayer.log.MyLog;
import com.chen.cyplayer.push.CyBasePushEncoder;
import com.chen.cyplayer.push.CyConnectListenr;
import com.chen.cyplayer.push.CyPushEncodec;
import com.chen.cyplayer.push.CyPushVideo;

public class LivePushActivity extends AppCompatActivity {

    private CyPushVideo cyPushVideo;
    private CyCameraView cyCameraView;
    private CyPushEncodec cyPushEncodec;
    private boolean start = false;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_live_push);
        cyCameraView = findViewById(R.id.cyCameraView);
        cyPushVideo = new CyPushVideo();

        cyPushVideo.setCyConnectListenr(new CyConnectListenr() {
            @Override
            public void onConnecting() {
                MyLog.d("链接服务器中..");
            }

            @Override
            public void onConnectSuccess() {
                MyLog.d( "链接服务器成功，可以开始推流了");
                cyPushEncodec = new CyPushEncodec(LivePushActivity.this, cyCameraView.getTextureId());
                cyPushEncodec.initEncodec(cyCameraView.getEglContext(), 720 / 2, 1280 / 2, 44100, 2);
                cyPushEncodec.startRecord();
                cyPushEncodec.setOnMediaInfoListener(new CyBasePushEncoder.OnMediaInfoListener() {
                    @Override
                    public void onMediaTime(int times) {

                    }

                    @Override
                    public void onSPSPPSInfo(byte[] sps, byte[] pps) {
                        cyPushVideo.pushSPSPPS(sps, pps);
                    }

                    @Override
                    public void onVideoInfo(byte[] data, boolean keyframe) {
                        cyPushVideo.pushVideoData(data, keyframe);
                    }
                });
            }

            @Override
            public void onConnectFail(String msg) {
                MyLog.d(msg);
            }
        });


    }

    public void push(View view) {
        start = !start;
        if (start){
            cyPushVideo.initLivePush("rtmp://send3a.douyu.com/live/6441662rCTzoSAsu?wsSecret=e26f45105bc61f8d7329ebeda86c1cc7&wsTime=5d4ac28f&wsSeek=off&wm=0&tw=0&roirecognition=0");
        }else {
            if(cyPushEncodec != null)
            {
                cyPushEncodec.stopRecord();
                cyPushEncodec = null;
            }
        }
    }
}
