package com.chen.cjsplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.ViewGroup;
import android.widget.LinearLayout;

import com.chen.cjsplayer.opengl.CyGLTextureView;
import com.chen.cjsplayer.opengl.CyMutiSurfaceVeiw;
import com.chen.cjsplayer.opengl.CyTextureRender;


public class TestActivity extends AppCompatActivity {
    private CyGLTextureView surfaceView;
    private LinearLayout lyContent;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_test);
        surfaceView = findViewById(R.id.surfaceView);
        lyContent = findViewById(R.id.ly_content);

        surfaceView.getCyTextureRender().setOnRenderCreateListener(new CyTextureRender.OnRenderCreateListener() {
            @Override
            public void create(final int textureId) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {

                        if (lyContent.getChildCount() > 0){
                            lyContent.removeAllViews();
                        }

                        for (int i = 0; i < 3; i++) {
                            CyMutiSurfaceVeiw view1 = new CyMutiSurfaceVeiw(TestActivity.this);
                            view1.setTextureId(textureId, i);
                            view1.setSurfaceAndEglContext(null, surfaceView.getEglContext());
                            LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);
                            lp.width = 200;
                            lp.height = 300;
                            view1.setLayoutParams(lp);
                            lyContent.addView(view1);
                        }
                    }
                });
            }
        });
    }
}
