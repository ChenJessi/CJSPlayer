package com.chen.cyplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.content.pm.ActivityInfo;
import android.hardware.Camera;
import android.media.MediaFormat;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.widget.Button;

import com.chen.cyplayer.camera.CyCameraView;
import com.chen.cyplayer.encodec.CyBaseMediaEncoder;
import com.chen.cyplayer.encodec.CyMediaEncodec;
import com.chen.cyplayer.log.MyLog;
import com.luck.picture.lib.PictureSelector;
import com.luck.picture.lib.config.PictureConfig;
import com.luck.picture.lib.config.PictureMimeType;

public class Main2Activity extends AppCompatActivity {
    private CyCameraView cyCameraView;
    private Button button;
    private CyMediaEncodec cyMediaEncodec;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main2);

        cyCameraView = findViewById(R.id.surfaceView);
        button = findViewById(R.id.button);
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (cyMediaEncodec == null) {
                    cyMediaEncodec = new CyMediaEncodec(Main2Activity.this, cyCameraView.getTextureId());
                    cyMediaEncodec.initEncodec(cyCameraView.getEglContext(),
                            Environment.getExternalStorageDirectory().getAbsolutePath() + "/wl_live_pusher.mp4", MediaFormat.MIMETYPE_VIDEO_AVC, 1080, 2280);
                    cyMediaEncodec.setOnMediaInfoListener(new CyBaseMediaEncoder.OnMediaInfoListener() {
                        @Override
                        public void onMediaTime(int times) {
                            Log.d("test", "time is : " + times);
                        }
                    });
                    cyMediaEncodec.startRecord();
                    button.setText("正在录制");
                }else {
                    cyMediaEncodec.stopRecord();
                    button.setText("开始录制");
                    cyMediaEncodec = null;
                }
            }
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        cyCameraView.onDestory();
    }
}
