package com.chen.cyplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.media.MediaFormat;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import com.chen.cyplayer.camera.CyCameraView;
import com.chen.cyplayer.encodec.CyBaseMediaEncoder;
import com.chen.cyplayer.encodec.CyMediaEncodec;
import com.chen.cyplayer.log.MyLog;
import com.chen.cyplayer.util.DisplayUtil;

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
                    MyLog.d("width : "+ DisplayUtil.getScreenWidth(Main2Activity.this) + "   height : "+DisplayUtil.getScreenHeight(Main2Activity.this));
                    Toast.makeText(Main2Activity.this, "width : "+ DisplayUtil.getScreenWidth(Main2Activity.this), Toast.LENGTH_SHORT).show();
                    cyMediaEncodec = new CyMediaEncodec(Main2Activity.this, cyCameraView.getTextureId());
                    cyMediaEncodec.initEncodec(cyCameraView.getEglContext(),
                            Environment.getExternalStorageDirectory().getAbsolutePath() + "/wl_live_pusher.mp4", MediaFormat.MIMETYPE_VIDEO_AVC, 720, 1280);

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

}
