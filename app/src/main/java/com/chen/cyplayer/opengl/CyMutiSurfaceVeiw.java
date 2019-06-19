package com.chen.cyplayer.opengl;

import android.content.Context;
import android.util.AttributeSet;

/**
 * @author Created by CHEN on 2019/6/19
 * @email 188669@163.com
 */
public class CyMutiSurfaceVeiw extends CyEGLSurfaceView{
    private CyMutiRender render;
    public CyMutiSurfaceVeiw(Context context) {
        this(context, null);
    }

    public CyMutiSurfaceVeiw(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public CyMutiSurfaceVeiw(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        render = new CyMutiRender(context);
        setRender(render);
    }

    public void setTextureId(int textureId, int index){
        if (render != null){
            render.setTextureId(textureId, index);
        }
    }
}
