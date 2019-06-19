package com.chen.cyplayer.opengl;

import android.content.Context;
import android.util.AttributeSet;

/**
 * @author Created by CHEN on 2019/6/19
 * @email 188669@163.com
 */
public class CyGLTextureView extends CyEGLSurfaceView {

    private CyTextureRender render;
    public CyGLTextureView(Context context) {
        this(context,  null);
    }

    public CyGLTextureView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public CyGLTextureView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        render = new CyTextureRender(context);
        setRender(render);
    }
    public CyTextureRender getCyTextureRender(){
        return render;
    }
}
