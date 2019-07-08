package com.chen.cyplayer.imgvideo;

import android.content.Context;
import android.util.AttributeSet;

import com.chen.cyplayer.opengl.CyEGLSurfaceView;
import com.chen.cyplayer.opengl.FboRender;

/**
 * @author Created by CHEN on 2019/7/8
 * @email 188669@163.com
 */
public class CyImgVideoView extends CyEGLSurfaceView {
    private CyImgVideoRender cyImgVideoRender;
    private int fbotextureid;

    public CyImgVideoView(Context context) {
        this(context, null);
    }

    public CyImgVideoView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public CyImgVideoView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        cyImgVideoRender = new CyImgVideoRender(context);
        setRender(cyImgVideoRender);
        setRenderMode(CyEGLSurfaceView.RENDERMODE_WHEN_DIRTY);
        cyImgVideoRender.setOnRenderCreateListener(new CyImgVideoRender.OnRenderCreateListener() {
            @Override
            public void onCreate(int textid) {
                fbotextureid = textid;
            }
        });
    }

    public void setCurrentImg(int imgsr) {
        if (cyImgVideoRender != null) {
            cyImgVideoRender.setCurrentImgSrc(imgsr);
            requestRender();
        }
    }

    public int getFbotextureid() {
        return fbotextureid;
    }
}
