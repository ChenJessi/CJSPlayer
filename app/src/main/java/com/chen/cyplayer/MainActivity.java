package com.chen.cyplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.os.Looper;
import android.util.Log;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;

import com.chen.cyplayer.bean.CyTimeInfoBean;
import com.chen.cyplayer.enums.MuteEnum;
import com.chen.cyplayer.listener.CyOnCompleteListener;
import com.chen.cyplayer.listener.CyOnLoadListener;
import com.chen.cyplayer.listener.CyOnPreparedListener;
import com.chen.cyplayer.listener.CyOnPcmInfoListener;
import com.chen.cyplayer.listener.CyOnTimeInfoListener;
import com.chen.cyplayer.listener.CyOnValumeDBListener;
import com.chen.cyplayer.log.MyLog;
import com.chen.cyplayer.opengl.CyGLSurfaceView;
import com.chen.cyplayer.player.CyPlayer;
import com.chen.cyplayer.util.CyTimeUtil;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    private CyPlayer cyPlayer;
    private TextView tvTime;
    private TextView tvVolum;
    private SeekBar seekVolume;
    private SeekBar seekSeek;
    private boolean isSeekBar = false;
    private int position;

    private boolean isCut = false;
    private CyGLSurfaceView surfaceView;
//    https://res.exexm.com/cw_145225549855002
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        tvTime = findViewById(R.id.tvTime);
        tvVolum = findViewById(R.id.tvVolum);
        seekVolume = findViewById(R.id.seekVolume);
        seekSeek = findViewById(R.id.seekSeek);
        surfaceView = findViewById(R.id.surfaceView);
//        cyPlayer = CyPlayer.getInstance();
        cyPlayer = new CyPlayer();
        cyPlayer.setCyGLSurfaceView(surfaceView);
        seekVolume.setProgress(cyPlayer.getVolumePercent());
        tvVolum.setText("音量:" + cyPlayer.getVolumePercent()+"");
        cyPlayer.setCyOnPreparedListener(new CyOnPreparedListener() {
            @Override
            public void onPrepared() {
                MyLog.d("准备好了，可以开始播放声音了！  ");
               cyPlayer.start();

            }
        });
        cyPlayer.setCyOnLoadListener(new CyOnLoadListener() {
            @Override
            public void onLoad(boolean load) {
                if (load){
                    MyLog.d("加载中...  ");
                }else {
                    MyLog.d("播放中...  ");
                }
            }
        });
        cyPlayer.setCyOnTimeInfoListener(new CyOnTimeInfoListener() {
            @Override
            public void timeInfo(CyTimeInfoBean timeInfoBean) {
                if (!isSeekBar && timeInfoBean.getTotalTime() > 0){
                    seekSeek.setProgress(timeInfoBean.getCurrentTime() * 100 / timeInfoBean.getTotalTime()  );
                }

                tvTime.setText(CyTimeUtil.secdsToDateFormat(timeInfoBean.getCurrentTime()) + "/" + CyTimeUtil.secdsToDateFormat(timeInfoBean.getTotalTime()));
            }
        });

        cyPlayer.setCyOnCompleteListener(new CyOnCompleteListener() {
            @Override
            public void onComplete() {
                MyLog.d("播放完成");
//                cyPlayer.playNext("/mnt/sdcard/tencent/QQfile_recv/许嵩+-+千古.ape");
            }
        });

        seekSeek.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (cyPlayer.getDuration() > 0 && fromUser){
                    position = cyPlayer.getDuration() * progress / 100;
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                isSeekBar = true;
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                isSeekBar = false;
                cyPlayer.seek(position);
            }
        });

        seekVolume.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                cyPlayer.setVolume(progress);
                tvVolum.setText("音量:" + cyPlayer.getVolumePercent()+"");
//                MyLog.d("volmue: "+cyPlayer.getVolumePercent());
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        cyPlayer.setCyOnValumeDBListener(new CyOnValumeDBListener() {
            @Override
            public void onDbValue(int db) {
//                MyLog.d("dbvalue : " + db);
            }
        });

        cyPlayer.setCyOnPcmInfoListener(new CyOnPcmInfoListener() {
            @Override
            public void onPcmInfo(int samplesize, byte[] buffer) {
//                MyLog.d("samplesize: "+samplesize +"  buffer"+buffer);
            }

            @Override
            public void onPcmRate(int samplerate, int bit, int channels) {

            }
        });
    }

    public void begin(View view) {

//        cyPlayer.setSource("/mnt/sdcard/tencent/QQfile_recv/几个你_薛之谦.aac");
        cyPlayer.setSource("/mnt/sdcard/tencent/QQfile_recv/测试.mp4");
        cyPlayer.prepared();

    }

    public void pause(View view) {
        cyPlayer.pause();
    }

    public void resume(View view) {
        cyPlayer.resume();
    }

    public void stop(View view) {
        cyPlayer.stop();
    }

    public void next(View view) {
//        for (int i = 0; i  <= 1000; i++){
//            Log.e("music", "next : "+i);
//            try {
//                Thread.sleep(1000);
//                Log.e("music", "next : "+i);
//                cyPlayer.playNext("/mnt/sdcard/tencent/QQfile_recv/许嵩+-+千古.ape");
//            } catch (InterruptedException e) {
//                e.printStackTrace();
//            }
//        }
//        cyPlayer.playNext("/mnt/sdcard/tencent/QQfile_recv/许嵩+-+千古.ape");
    }

    public void right(View view) {
        cyPlayer.setMute(MuteEnum.MUTE_CENTER);
    }

    public void left(View view) {
        cyPlayer.setMute(MuteEnum.MUTE_LEFT);
    }

    public void center(View view) {
        cyPlayer.setMute(MuteEnum.MUTE_CENTER);
    }

    public void normal(View view) {
        cyPlayer.setPitch(1.0f);
        cyPlayer.setSpeed(1.0f);
    }

    public void pitch(View view) {
        cyPlayer.setPitch(2.0f);
        cyPlayer.setSpeed(1.0f);
    }

    public void speed(View view) {
        cyPlayer.setPitch(1.0f);
        cyPlayer.setSpeed(2.0f);
    }

    public void pitchSpeed(View view) {
        cyPlayer.setPitch(2.0f);
        cyPlayer.setSpeed(2.0f);
    }

    public void startRecord(View view) {
        cyPlayer.startRecord(new File("/mnt/sdcard/tencent/QQfile_recv/test1.aac"));
    }

    public void startCut(View view) {
//        isCut = true;
//        cyPlayer.cutAudioPlay(20,40 );
    }

    public void stopRecord(View view) {
//        cyPlayer.stopRecord();
    }

    public void pauseRecord(View view) {
//        cyPlayer.pauseRecord();
    }

    public void resumeRcord(View view) {
//        cyPlayer.resumeRcord();
    }
}
