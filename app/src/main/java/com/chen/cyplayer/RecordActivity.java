package com.chen.cyplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;

import com.chen.cyplayer.audiorecord.AudioRecordUitl;
import com.chen.cyplayer.log.MyLog;

public class RecordActivity extends AppCompatActivity {

    private AudioRecordUitl audioRecordUitl;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_record);
    }

    public void start(View view) {

        if (audioRecordUitl == null) {
            audioRecordUitl = new AudioRecordUitl();
            audioRecordUitl.setOnRecordListener(new AudioRecordUitl.OnRecordListener() {
                @Override
                public void recordByte(byte[] audioData, int readSize) {
                    MyLog.d("readSize : " + readSize);
                }
            });
            audioRecordUitl.startRecord();
        } else {
            audioRecordUitl.stopRecord();
            audioRecordUitl = null;
        }
    }
}
